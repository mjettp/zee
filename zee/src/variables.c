/* Variables handling functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id$	*/

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zee.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"

/*
 * Default variables values table.
 */
static struct var_entry {
  char *var;	/* Variable name. */
  char *fmt;	/* Variable format (boolean, etc.). */
  char *val;	/* Default value. */
} def_vars[] = {
#define X(zee_var, fmt, val, doc) { zee_var, fmt, val },
#include "tbl_vars.h"
#undef X
};

void init_variables(void)
{
  struct var_entry *p;

  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    set_variable(p->var, p->val);
}

void set_variable(char *var, char *val)
{
  variableSetString(&mainVarList, var, val);
}

char *get_variable_bp(Buffer *bp, char *var)
{
  char *s = NULL;

  if (bp)
    s = variableGetString(bp->vars, var);

  if (s == NULL)
    s = variableGetString(mainVarList, var);

  return s;
}

char *get_variable(char *var)
{
  return get_variable_bp(cur_bp, var);
}

int get_variable_number_bp(Buffer *bp, char *var)
{
  int t = 0;
  char *s = get_variable_bp(bp, var);

  if (s)
    t = atoi(s);

  return t;
}

int get_variable_number(char *var)
{
  return get_variable_number_bp(cur_bp, var);
}

int is_variable_equal(char *var, char *val)
{
  char *v = get_variable(var);
  return v != NULL && !strcmp(v, val);
}

int lookup_bool_variable(char *var)
{
  char *p;

  if ((p = get_variable(var)) != NULL)
    return !strcmp(p, "true");

  return FALSE;
}

char *minibuf_read_variable_name(char *msg)
{
  char *ms;
  Completion *cp = completion_new(FALSE);
  le *lp;

  for (lp = mainVarList; lp != NULL; lp = lp->list_next)
    list_append(cp->completions, zstrdup(lp->data));

  for (;;) {
    ms = minibuf_read_completion(msg, "", cp, NULL);

    if (ms == NULL) {
      free_completion(cp);
      cancel();
      return NULL;
    }

    if (ms[0] == '\0') {
      free_completion(cp);
      minibuf_error("No variable name given");
      return NULL;
    } else if (get_variable(ms) == NULL) {
      minibuf_error("Undefined variable name `%s'", ms);
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  free_completion(cp);

  return ms;
}

static char *get_variable_format(char *var)
{
  struct var_entry *p;
  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    if (!strcmp(p->var, var))
      return p->fmt;

  return "";
}

DEFUN_INT("set-variable", set_variable)
  /*+
    Set a variable value to the user-specified value.
    +*/
{
  char *var, *val, *fmt;

  var = minibuf_read_variable_name("Set variable: ");
  if (var == NULL)
    return FALSE;

  fmt = get_variable_format(var);
  if (!strcmp(fmt, "b")) {
    int i;
    if ((i = minibuf_read_boolean("Set %s to value: ", var)) == -1)
      return cancel();
    val = (i == TRUE) ? "true" : "false";
  } else { /* Non-boolean variable. */
    if ((val = minibuf_read("Set %s to value: ", "", var)) == NULL)
      return cancel();
  }

  /* `tab-width' and `fill-column' automatically become
     buffer-local when set in any fashion. */
  if (!strcmp(var, "tab-width") || !strcmp(var, "fill-column"))
    variableSetString(&cur_bp->vars, var, val);
  else
    set_variable(var, val);

  return TRUE;
}
END_DEFUN