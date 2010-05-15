/* Undo facility functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Undo action
 */
struct Undo
{
  /* Next undo delta in list. */
  Undo *next;

  /* The type of undo delta. */
  int type;

  /* Where the undo delta need to be applied.
     Warning!: Do not use the p field of pt. */
  int pt;

  /* Flag indicating that reverting this undo leaves the buffer
     in an unchanged state. */
  bool unchanged;

  /* The block to insert. */
  astr text;
  size_t osize;		/* Original size. */
  size_t size;		/* New block size. */
};

/* Setting this variable to true stops undo_save saving the given
   information. */
int undo_nosave = false;

/* This variable is set to true when an undo is in execution. */
static int doing_undo = false;

/*
 * Save a reverse delta for doing undo.
 */
void
undo_save (int type, int pt, size_t osize, size_t size)
{
  Undo *up;

  if (get_buffer_noundo (cur_bp) || undo_nosave)
    return;

  up = (Undo *) XZALLOC (Undo);
  up->type = type;
  up->pt = point_copy (pt);
  if (!get_buffer_modified (cur_bp))
    up->unchanged = true;

  if (type == UNDO_REPLACE_BLOCK)
    {
      up->osize = osize;
      up->size = size;
      up->text = copy_text_block (point_copy (pt), osize);
    }

  up->next = get_buffer_last_undop (cur_bp);
  set_buffer_last_undop (cur_bp, up);

  if (!doing_undo)
    set_buffer_next_undop (cur_bp, up);
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo *
revert_action (Undo * up)
{
  size_t i;

  doing_undo = true;

  if (up->type == UNDO_END_SEQUENCE)
    {
      undo_save (UNDO_START_SEQUENCE, up->pt, 0, 0);
      up = up->next;
      while (up->type != UNDO_START_SEQUENCE)
        up = revert_action (up);
      undo_save (UNDO_END_SEQUENCE, up->pt, 0, 0);
      goto_point (up->pt);
      return up->next;
    }

  goto_point (up->pt);

  if (up->type == UNDO_REPLACE_BLOCK)
    {
      undo_save (UNDO_REPLACE_BLOCK, up->pt, up->size, up->osize);
      undo_nosave = true;
      for (i = 0; i < up->size; ++i)
        delete_char ();
      insert_nstring (astr_cstr (up->text), up->osize);
      undo_nosave = false;
    }

  doing_undo = false;

  if (up->unchanged)
    set_buffer_modified (cur_bp, false);

  return up->next;
}

DEFUN ("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
  if (get_buffer_noundo (cur_bp))
    {
      minibuf_error ("Undo disabled in this buffer");
      return leNIL;
    }

  if (warn_if_readonly_buffer ())
    return leNIL;

  if (get_buffer_next_undop (cur_bp) == NULL)
    {
      minibuf_error ("No further undo information");
      set_buffer_next_undop (cur_bp, get_buffer_last_undop (cur_bp));
      return leNIL;
    }

  set_buffer_next_undop (cur_bp, revert_action (get_buffer_next_undop (cur_bp)));
  minibuf_write ("Undo!");
}
END_DEFUN

void
free_undo (Undo *up)
{
  while (up != NULL)
    {
      Undo *next_up = up->next;
      if (up->type == UNDO_REPLACE_BLOCK)
        astr_delete (up->text);
      free (up);
      up = next_up;
    }
}

/*
 * Set unchanged flags to false.
 */
void
undo_set_unchanged (Undo *up)
{
  for (; up; up = up->next)
    up->unchanged = false;
}
