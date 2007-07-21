/* Keyboard functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2006 Reuben Thomas.
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

#include "config.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "term.h"
#include "extern.h"


// Get keystrokes

#define MAX_KEY_BUF	16

static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t xgetkey(int mode, size_t timeout)
{
  size_t key;

  if (keyp > key_buf)
    return *--keyp;

  key = term_xgetkey(mode, timeout);
  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd(key);
  return key;
}

/*
 * Wait for a keystroke indefinitely, and return the
 * corresponding keycode.
 */
size_t getkey(void)
{
  return xgetkey(0, 0);
}

/*
 * Wait for timeout 10ths if a second or until a key is pressed.
 * The key is then available with [x]getkey().
 */
void waitkey(size_t timeout)
{
  ungetkey(xgetkey(GETKEY_DELAYED, timeout));
}

void ungetkey(size_t key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}


// Key code and name arrays

static size_t keycode[] = {
#define X(key_sym, key_name, key_code) \
	key_sym,
#include "tbl_keys.h"
#undef X
};

static const char *keyname[] = {
#define X(key_sym, key_name, key_code) \
	key_name,
#include "tbl_keys.h"
#undef X
};

/*
 * Convert a key chord into its ASCII representation
 */
rblist chordtostr(size_t key)
{
  bool found;
  size_t i;
  rblist as = rblist_empty;

  if (key & KBD_CTRL)
    as = rblist_concat(as, rblist_from_string("C-"));
  if (key & KBD_META)
    as = rblist_concat(as, rblist_from_string("M-"));
  key &= ~(KBD_CTRL | KBD_META);

  for (found = false, i = 0; i < sizeof(keycode) / sizeof(keycode[0]); i++)
    if (keycode[i] == key) {
      as = rblist_concat(as, rblist_from_string(keyname[i]));
      found = true;
      break;
    }

  if (found == false) {
    if (isgraph(key))
      as = rblist_append(as, (int)(key & 0xff));
    else
      as = rblist_concat(as, rblist_afmt("<%x>", key));
  }

  return as;
}

/*
 * Convert a key name to its key code
 */
// FIXME: This seems to match any entry 'i' where 'name' is a prefix of
// 'keyname[i]' or vice-versa.
// Matching when 'keyname[i]' is a prefix of 'name' is intended, but matching
// when 'name' is a prefix of 'keyname[i]' is not. (Intention inferred from
// the sole caller which is strtochord below). Therefore there are actually
// two bugs here: one in the code and one in the comment.
static size_t strtokey(rblist name, size_t *len)
{
  size_t i;

  for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    size_t keylen = strlen(keyname[i]);
    if (strncmp(rblist_to_string(name), keyname[i], min(keylen, rblist_length(name))) == 0) {
      *len = keylen;
      return keycode[i];
    }
  }

  *len = 1;
  return (size_t)rblist_get(name, 0);
}

/*
 * Convert a key chord string to its key code
 */
size_t strtochord(rblist chord)
{
  size_t key = 0, len = 0, k;

  do {
    size_t l;
    k = strtokey(rblist_sub(chord, len, rblist_length(chord)), &l);
    key |= k;
    len += l;
  } while (k == KBD_CTRL || k == KBD_META);

  if (len != rblist_length(chord))
    key = KBD_NOKEY;

  return key;
}
