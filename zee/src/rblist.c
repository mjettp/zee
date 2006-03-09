/* Randomly balanced lists.
   Copyright (c) 2006 Alistair Turnbull.
   Copyright (c) 2006 Reuben Thomas.
   All rights reserved.

   This file is part of Zee.

   Zee is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zee is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zee; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "zmalloc.h"
#include "rblist.h"


/* For debugging: incremented every time random_float is called. */
static size_t random_counter = 0;

/* Returns a random double in the range 0.0 (inclusive) to 1.0 (exclusive). */
static inline double random_double(void)
{
  static uint32_t seed = 483568341; /* Arbitrary. */

  random_counter++;
  seed = (2 * seed - 1) * seed + 1; /* Maximal period mod any power of two. */
  return seed * (1.0 / (1.0 + (double)UINT32_MAX + 1.0));
}

/* Returns a random size_t in the range from 1 to 'n'. */
static inline size_t random_one_to_n(size_t n)
{
  return 1 + (size_t)(n * random_double());
}

/*
 * Tuning parameter which controls memory/CPU compromise.
 * rblists shorter than this will be implemented using a primitive array.
 * Increase to use less memory and more CPU.
 */
#define MINIMUM_NODE_LENGTH 32

/*
 * In ML, this data structure would be defined as follows:
 *
 *   datatype rblist =
 *     | leaf of int * int * char array
 *     | node of int * int * rblist * rblist;
 *
 * "leaf" represents a primitive array, and "node" represents the
 * concatenation of two shorter lists. In both cases, the first int field
 * is the length of the list and the second is the number of newline
 * characters it contains.
 *
 * In C, we can distinguish "leaf" from "node" by comparing the length
 * with MINIMUM_NODE_LENGTH. We can therefore use an untagged
 * union. To allow us to access the length and newline count without first
 * knowing whether it is a leaf or a node, we include a third member
 * of the union.
 *
 * The subtle thing about randomly balanced lists is that they are not
 * completely deterministic; the arrangement of leafs and nodes is chosen
 * at random from the set of all possible structures that represent the
 * correct list.
 *
 * The only operation which truly chooses the whole structure randomly is
 * rblist_from_array. In other operations the structure of the result is
 * correlated with the structure of the operands. The result has the
 * correct probability distribution only on the assumption that the
 * operands have the correct probability distribution and are independent.
 */

struct leaf {
  size_t length;
  size_t nl_count;
  char data[];
};

struct node {
  size_t length;
  size_t nl_count;
  rblist left;
  rblist right;
};

/* This is the opaque public type. */
union rblist {
  struct {
    size_t length;
    size_t nl_count;
  } stats;
  struct leaf leaf;
  struct node node;
};

/*
 * In ML this data structure would be leaf * int * rblist list. The leaf
 * is the current struct leaf. The int is the current position within
 * that struct leaf. The rblist list is a linked list of rblists to
 * iterate over after that.
 *
 * In C, we have to define a struct for the links in the linked list. The
 * list is terminated by a NULL next pointer.
 */

struct link {
  rblist item;
  const struct link *next;
};

/* This is the opaque public type. */
struct rblist_iterator {
  const struct leaf *leaf;
  size_t pos;
  const struct link *next;
};

/*****************************/
/* Static utility functions. */

/* The struct leaf to which rblist_empty points. */
static const struct leaf empty = {0, 0};

/* Allocates and initialises a struct leaf. */
static inline rblist leaf_from_array(const char *s, size_t length)
{
  assert(length < MINIMUM_NODE_LENGTH);
  struct leaf *ret = zmalloc(sizeof(struct leaf) + length * sizeof(char));
  ret->length = length;
  ret->nl_count = 0;
  for (size_t i=0; i<length; i++)
    if (s[i] == '\n')
      ret->nl_count++;
  memcpy(&ret->data[0], s, length);
  return (rblist)ret;
}

/* Tests whether an rblist is a leaf or a node. */
static inline bool is_leaf(rblist rbl)
{
  return rbl->stats.length < MINIMUM_NODE_LENGTH;
}

/*
 * Used internally by rblist_concat.
 *
 * Splits 'rbl' into two halves and stores those halves in *left and
 * *right. If is_leaf(rbl), the split is made at position 'pos', which
 * must be chosen from a uniform probability distribution over
 * 0 < pos < rbl->length. If !is_leaf(rbl), the split is made
 * in the same place that 'rbl' was split last time, i.e. the two halves
 * are rbl->node.left and rbl->node.right.
 *
 * Either way, the split happens at a random position chosen from a
 * uniform probability distribution.
 */
static inline void random_split(rblist rbl, size_t pos, rblist *left, rblist *right)
{
  if (is_leaf(rbl)) {
    *left = leaf_from_array(&rbl->leaf.data[0], pos);
    *right = leaf_from_array(&rbl->leaf.data[pos], rbl->leaf.length - pos);
  } else {
    *left = rbl->node.left;
    *right = rbl->node.right;
  }
}

/*
 * Used internally by recursive_split.
 *
 * Makes a struct leaf or struct node as appropriate to represent the
 * concatenation of `left' and `right'. In the node case, `left' and
 * `right' become the children.
 */
static rblist make_rblist(rblist left, rblist right)
{
  size_t new_len = left->stats.length + right->stats.length;
  if (new_len < MINIMUM_NODE_LENGTH) {
    struct leaf *ret = zmalloc(sizeof(struct leaf) + sizeof(char) * new_len);
    ret->length = new_len;
    ret->nl_count = left->leaf.nl_count + right->leaf.nl_count;
    memcpy(&ret->data[0], &left->leaf.data[0], left->leaf.length);
    memcpy(&ret->data[left->leaf.length], &right->leaf.data[0], right->leaf.length);
    return (rblist)ret;
  } else {
    struct node *ret = zmalloc(sizeof(struct node));
    ret->length = new_len;
    ret->nl_count = left->stats.nl_count + right->stats.nl_count;
    ret->left = left;
    ret->right = right;
    return (rblist)ret;
  }
}

/* The recursive part of rblist_split. */
static void recursive_split(rblist rbl, size_t pos, rblist *left, rblist *right)
{
  /* FIXME: Rewrite using a while loop instead of recursion? Not easy
   * because of the `if' in make_rblist. */

  if (is_leaf(rbl)) {
    *left = leaf_from_array(&rbl->leaf.data[0], pos);
    *right = leaf_from_array(&rbl->leaf.data[pos], rbl->leaf.length - pos);
    return;
  }
  size_t mid = rbl->node.left->stats.length;
  if (pos < mid) {
    rblist left_right;
    rblist_split(rbl->node.left, pos, left, &left_right);
    *right = make_rblist(left_right, rbl->node.right);
  } else if (pos > mid) {
    rblist right_left;
    rblist_split(rbl->node.right, pos - mid, &right_left, right);
    *left = make_rblist(rbl->node.left, right_left);
  } else {
    *left = rbl->node.left;
    *right = rbl->node.right;
  }
}

/*
 * Constructs an iterator that iterates over 'rbl' and then over all the
 * rblists in 'next'.
 */
static rblist_iterator make_iterator(rblist rbl, const struct link *next)
{
  while (!is_leaf(rbl)) {
    struct link *link = zmalloc(sizeof(struct link));
    link->item = rbl->node.right;
    link->next = next;
    next = link;
    rbl = rbl->node.left;
  }
  rblist_iterator ret = zmalloc(sizeof(struct rblist_iterator));
  ret->leaf = &rbl->leaf;
  ret->pos = 0;
  ret->next = next;
  return ret;
}

/***************************/
/* Primitive constructors. */

/*
 * This operation is unique in picking the entire node/leaf structure
 * completely at random. It can therefore be thought of as defining the
 * correct probability distribution for randomly balanced lists.
 *
 * It's not really a necessary primitive constructor, but the above
 * property argues for implementing it as a primitive.
 */
rblist rblist_from_array(const char *s, size_t length)
{
  if (length == 0)
    return rblist_empty;
  if (length < MINIMUM_NODE_LENGTH)
    return leaf_from_array(s, length);
  struct node *ret = zmalloc(sizeof(struct node));
  ret->length = length;
  size_t pos = random_one_to_n(length - 1);
  ret->left = rblist_from_array(&s[0], pos);
  ret->right = rblist_from_array(&s[pos], length - pos);
  ret->nl_count = ret->left->stats.nl_count + ret->right->stats.nl_count;
  return (rblist)ret;
}

const rblist rblist_empty = (rblist)&empty;

rblist rblist_singleton(char c)
{
  return leaf_from_array(&c, 1);
}

/*
 * Balancing condition:
 *
 *   rblist_concat(rblist_from_array(left), rblist_from_array(right))
 *
 * ... has the same probability distribution as:
 *
 *   rblist_from_array(concatenation_of_left_and_right)
 *
 * Strictly, left and right must be uncorrelated for this balancing
 * condition to hold, but in practice concatenating a list with itself
 * (for example) does not have disastrous consequences.
 */
rblist rblist_concat(rblist left, rblist right)
{
  /* FIXME: Rewrite using a while loop instead of recursion. */

  #define llen (left->stats.length)
  #define rlen (right->stats.length)
  #define mid (left->stats.length)
  size_t new_len = llen + rlen;

  if (new_len == 0)
    return rblist_empty;
  else if (new_len < MINIMUM_NODE_LENGTH) {
    struct leaf *ret = zmalloc(sizeof(struct leaf) + sizeof(char) * new_len);
    ret->length = new_len;
    ret->nl_count = left->leaf.nl_count + right->leaf.nl_count;
    memcpy(&ret->data[0], &left->leaf.data[0], llen);
    memcpy(&ret->data[mid], &right->leaf.data[0], rlen);
    return (rblist)ret;
  } else {
    struct node *ret = zmalloc(sizeof(struct node));
    ret->length = new_len;
    ret->nl_count = left->stats.nl_count + right->stats.nl_count;
    size_t pos = random_one_to_n(new_len - 1);
    if (pos < mid) {
      random_split(left, pos, &ret->left, &left);
      ret->right = rblist_concat(left, right);
    } else if (pos > mid) {
      random_split(right, pos - mid, &right, &ret->right);
      ret->left = rblist_concat(left, right);
    } else { /* pos == mid */
      ret->left = left;
      ret->right = right;
    }
    return (rblist)ret;
  }

  #undef llen
  #undef rlen
  #undef mid
}

/*************************/
/* Derived constructors. */

rblist rblist_from_string(const char *s)
{
  return rblist_from_array(s, strlen(s));
}

/**************************/
/* Primitive destructors. */

size_t rblist_length(rblist rbl)
{
  return rbl->stats.length;
}

size_t rblist_nl_count(rblist rbl)
{
  return rbl->stats.nl_count;
}

void rblist_split(rblist rbl, size_t pos, rblist *left, rblist *right)
{
  if (pos <= 0) {
    *left = rblist_empty;
    *right = rbl;
  } else if (pos >= rbl->stats.length) {
    *left = rbl;
    *right = rblist_empty;
  } else
    recursive_split(rbl, pos, left, right);
}

rblist_iterator rblist_iterate(rblist rbl)
{
  return rbl->stats.length ? make_iterator(rbl, NULL) : NULL;
}

char rblist_iterator_value(rblist_iterator it)
{
  return it->leaf->data[it->pos];
}

rblist_iterator rblist_iterator_next(rblist_iterator it)
{
  if (++it->pos < it->leaf->length)
    return it;
  return it->next ? make_iterator(it->next->item, it->next->next) : NULL;
}

/************************/
/* Derived destructors. */

char *rblist_copy_into(rblist rbl, char *s)
{
  RBLIST_FOR(c, rbl)
    *(s++) = c;
  RBLIST_END
  return s;
}

char *rblist_to_string(rblist rbl)
{
  char *s = zmalloc(rblist_length(rbl) + 1);
  *rblist_copy_into(rbl, s) = 0;
  return s;
}

rblist rblist_sub(rblist rbl, size_t from, size_t to)
{
  rblist dummy;
  rblist_split(rbl, to, &rbl, &dummy);
  rblist_split(rbl, from, &dummy, &rbl);
  return rbl;
}

/* FIXME: Write a garbage-free version? */
char rblist_get(rblist rbl, size_t pos)
{
  rblist dummy;
  rblist_split(rbl, pos, &dummy, &rbl);
  RBLIST_FOR(c, rbl)
    return c;
  RBLIST_END

  assert(0); /* pos > rblist_length(rbl). */
}

int rblist_compare(rblist left, rblist right)
{
  rblist_iterator it_left = rblist_iterate(left);
  rblist_iterator it_right = rblist_iterate(right);
  while (it_left && it_right) {
    if (rblist_iterator_value(it_left) != rblist_iterator_value(it_right))
      return  rblist_iterator_value(it_left) - rblist_iterator_value(it_right);
    it_left = rblist_iterator_next(it_left);
    it_right = rblist_iterator_next(it_right);
  }
  return it_left ? 1 : it_right ? -1 : 0;
}

#ifdef TEST

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "astr.h"

/* Stub to make zrealloc happy */
void die(int exitcode)
{
  exit(exitcode);
}

/* Print the lengths of a list into a string.
   Return a char * because that's always what we want for diagnostic
   messages below. */
static const char *rbl_structure(rblist rbl)
{
  if (is_leaf(rbl))
    return astr_cstr(astr_afmt("%d", rbl->leaf.length));
  else
    return astr_cstr(astr_afmt("(%s,%s)", rbl_structure(rbl->node.left),
                               rbl_structure(rbl->node.right)));
}

int main(void)
{
  rblist rbl1, rbl2, rbl3, rbl4;

  const char *s1 = "Hello, I'm longer than 32 characters! Whooooppeee!!! Yes, really, really long. You won't believe how incredibly enormously long I am!";
  /* Check that we'll have a whole number of steps in the loop below. */
  assert(strlen(s1) == 19 * 7);
  
  size_t count;

  /* Test length, for, to_string, concat for very short strings. */

  rbl1 = rblist_empty;
  assert(rblist_length(rbl1) == 0);
  RBLIST_FOR(c, rbl1)
    (void)c;
    assert(0);
  RBLIST_END
  assert(!strcmp(rblist_to_string(rbl1), ""));
  
  rbl1 = rblist_singleton('a');
  assert(rblist_length(rbl1) == 1);
  count = 0;
  RBLIST_FOR(c, rbl1)
    (void)c;
    assert(count++ < 1);
  RBLIST_END
  assert(!strcmp(rblist_to_string(rbl1), "a"));
  
  rbl1 = rblist_concat(rblist_singleton('a'), rblist_singleton('b'));
  assert(rblist_length(rbl1) == 2);
  count = 0;
  RBLIST_FOR(c, rbl1)
    (void)c;
    assert(count++ < 2);
  RBLIST_END
  assert(!strcmp(rblist_to_string(rbl1), "ab"));

  /* Test computational complexity, and test concat for long strings. */

  rbl1 = rblist_empty;
  rbl2 = rblist_singleton('x');
  #define TEST_SIZE 1000
  random_counter = 0;
  for (size_t i = 0; i < TEST_SIZE; i++)
    rbl1 = rblist_concat(rbl1, rbl2);
  assert(rblist_length(rbl1) == TEST_SIZE);
#ifdef DEBUG
  printf("Making a list of length %d by appending one element at a time required\n"
         "%d random numbers. Resulting structure is:\n", TEST_SIZE, random_counter);
  printf("%s\n", rbl_structure(rbl1));
#endif

  /* Test from_array, length and for for long strings. */

  rbl1 = rblist_from_array(s1, strlen(s1));
  count = 0;
  RBLIST_FOR(i, rbl1) {
#ifdef DEBUG
    printf("Element %d is '%c'\n", count, i);
#endif
    assert(i == s1[count]);
    count++;
  }
  RBLIST_END
  assert(count == strlen(s1));

  /* Test split and stress concat some more. */

  rbl1 = rblist_from_array(s1, strlen(s1));
#ifdef DEBUG
  printf("%s\n", rbl_structure(rbl1));
#endif
  assert(!strcmp(rblist_to_string(rbl1), s1));
  for (size_t i = 0; i <= rblist_length(rbl1); i += 19) {
    rblist_split(rbl1, i, &rbl2, &rbl3);
    assert(rblist_length(rbl2) == i);
    assert(rblist_length(rbl3) == 19 * 7 - i);
    rbl4 = rblist_concat(rbl2, rbl3);
    assert(rblist_length(rbl4) == 19 * 7);
#ifdef DEBUG
    printf("rbl2 = %s\nrbl3 = %s\n", rblist_to_string(rbl2), rblist_to_string(rbl3));
    printf("%s plus %s makes %s\n", rbl_structure(rbl2), rbl_structure(rbl3), rbl_structure(rbl4));
#endif
    assert(!strcmp(rblist_to_string(rbl1), rblist_to_string(rbl4)));
  }
  
  /* Test compare. */
  
  char *t[] = {"", "a", "b", "aa", "ab", "ba", "bb", NULL};
  for (int i = 0; t[i]; i++) for (int j = 0; t[j]; j++) {
    rbl1 = rblist_from_string(t[i]);
    rbl2 = rblist_from_string(t[j]);
    int cmp1 = rblist_compare(rbl1, rbl2);
    int cmp2 = strcmp(t[i], t[j]);
#ifdef DEBUG
    printf("rblist_compare(\"%s\", \"%s\") returns %d\n", t[i], t[j], cmp1);
#endif
    assert((cmp1 < 0) == (cmp2 < 0));
    assert((cmp1 > 0) == (cmp2 > 0));
  }

  return EXIT_SUCCESS;
}

#endif /* TEST */