/* Vectors (auto-extending arrays)
   Copyright (c) 2004 Reuben Thomas.  All rights reserved.

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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id$	*/

#include <stddef.h>
#include <stdlib.h>

#include "config.h"
#include "vector.h"
#include "zee.h"
#include "extern.h"

/* Create a vector whose items' size is size */
vector *vec_new(size_t itemsize)
{
  vector *v = zmalloc(sizeof(vector));
  vec_itemsize(v) = itemsize;
  vec_items(v) = 0;
  v->size = 0;
  v->array = NULL;
  return v;
}

/* Free a vector v */
void vec_delete(vector *v)
{
  free(v->array);
  free(v);
}

/* Resize a vector v to items elements */
static vector *resize(vector *v, size_t items)
{
  v->size = items;
  v->array = zrealloc(v->array, items * vec_itemsize(v));
  return v;
}

/* Convert a vector to an array */
void *vec_toarray(vector *v) {
  void *a;
  resize(v, vec_items(v));
  a = v->array;
  free(v);
  return a;
}

/* Return the address of a vector element, growing the array if
   needed */
void *vec_index(vector *v, size_t idx)
{
  if (idx >= v->size)
    v = resize(v, idx >= v->size * 2 ? idx + 1 : v->size * 2);
  if (idx >= vec_items(v))
    vec_items(v) = idx + 1;
  return (void *)((char *)v->array + idx * vec_itemsize(v));
}
