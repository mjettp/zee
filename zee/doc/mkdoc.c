/*
 * Produce various documentation and header files
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub to make zrealloc happy */
#define die exit

/* #include other sources so this program can be easily built on the
   build host when cross-compiling */
#include "zmalloc.c"
#include "astr.c"
#include "vector.c"

#define NAME "mkdoc"

struct fentry {
  astr name;
  astr doc;
};

static vector *ftable;
static size_t fentries = 0;

static struct {
  char *name;
  char *fmt;
  char *defvalue;
  char *doc;
} vtable[] = {
#define X(name, fmt, defvalue, doc) \
	{name, fmt, defvalue, doc},
#include "tbl_vars.h"
#undef X
};
#define ventries (sizeof vtable / sizeof vtable[0])

static struct {
  char *longname;
  char *doc;
  int opt;
} otable[] = {
#define X(longname, doc, opt) \
        {longname, doc, opt},
#include "tbl_opts.h"
#undef X
};
#define oentries (sizeof otable / sizeof otable[0])

static void fdecl(FILE *fp, astr name)
{
  int state = 0;
  astr doc = astr_new(""), line;
  struct fentry func;

  while ((line = astr_fgets(fp)) != NULL) {
    if (state == 1) {
      if (strncmp(astr_cstr(line), "+*/", 3) == 0) {
        state = 2;
        break;
      }
      astr_cat(doc, line);
      astr_cat_char(doc, '\n');
    } else if (strncmp(astr_cstr(line), "/*+", 3) == 0)
      state = 1;
  }

  if (strcmp(astr_cstr(doc), "") == 0) {
    fprintf(stderr, NAME ": no docstring for %s\n", astr_cstr(name));
    exit(1);
  } else if (state == 1) {
    fprintf(stderr, NAME ": unterminated docstring for %s\n", astr_cstr(name));
    exit(1);
  }

  func.name = astr_dup(name);
  func.doc = doc;
  vec_item(ftable, fentries++, struct fentry) = func;
}

static void get_funcs(FILE *fp)
{
  astr buf;

  while ((buf = astr_fgets(fp)) != NULL) {
    const char *s = astr_cstr(buf);
    if (!strncmp(s, "DEFUN(", 6) ||
        !strncmp(s, "DEFUN_INT(", 10)) {
      char *p = strchr(s, '(');
      char *q = strrchr(s, ')');
      if (p == NULL || q == NULL || p == q) {
        fprintf(stderr, NAME ": invalid DEFUN() syntax\n");
        exit(1);
      }
      fdecl(fp, astr_sub(buf, (p - s) + 1, q - s));
    }
  }
}

static void dump_funcs(void)
{
  size_t i;
  FILE *fp1 = fopen("zee_funcs.texi", "w");
  FILE *fp2 = fopen("tbl_funcs.h", "w");

  assert(fp1);
  fprintf(fp1,
          "@c Automatically generated file: DO NOT EDIT!\n"
          "@table @code\n");

  assert(fp2);
  fprintf(fp2,
          "/*\n"
          " * Automatically generated file: DO NOT EDIT!\n"
          " * Table of commands (name)\n"
          " */\n"
          "\n");

  for (i = 0; i < fentries; ++i) {
    fprintf(fp1, "@item %s\n%s", astr_cstr(vec_item(ftable, i, struct fentry).name),
            astr_cstr(vec_item(ftable, i, struct fentry).doc));
    fprintf(fp2, "X(%s)\n", astr_cstr(vec_item(ftable, i, struct fentry).name));
  }

  fprintf(fp1, "@end table");
  fclose(fp1);
  fclose(fp2);
}

static void dump_help(void)
{
  size_t i;
  for (i = 0; i < fentries; ++i) {
    astr doc = vec_item(ftable, i, struct fentry).doc;
    if (doc)
      fprintf(stdout, "\fF_%s\n%s",
              astr_cstr(vec_item(ftable, i, struct fentry).name), astr_cstr(doc));
  }
  for (i = 0; i < ventries; ++i)
    fprintf(stdout, "\fV_%s\n%s\n%s\n",
            vtable[i].name, vtable[i].defvalue,
            vtable[i].doc);
}

static void dump_vars(void)
{
  size_t i;
  FILE *fp = fopen("zee_vars.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @code\n");

  for (i = 0; i < ventries; ++i) {
    astr doc = astr_new(vtable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for %s\n", vtable[i].name);
      exit(1);
    }
    fprintf(fp, "@item %s\n%s\n", vtable[i].name, astr_cstr(doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void dump_opts(void)
{
  size_t i;
  FILE *fp = fopen("zee_opts.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @samp\n");

  for (i = 0; i < oentries; ++i) {
    astr doc = astr_new(otable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for --%s\n", otable[i].longname);
      exit(1);
    }
    fprintf(fp, "@item --%s\n%s\n", otable[i].longname, astr_cstr(doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

int main(int argc, char **argv)
{
  int i;
  FILE *fp = NULL;

  ftable = vec_new(sizeof(struct fentry));
  for (i = 1; i < argc; i++) {
    if (argv[i] != NULL && strcmp(argv[i], "-") != 0 &&
        (fp = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, NAME ":%s: %s\n",
              argv[i], strerror(errno));
      exit(1);
    }
    get_funcs(fp);
    fclose(fp);
  }
  dump_funcs();

  dump_help();
  dump_vars();
  dump_opts();

  return 0;
}
