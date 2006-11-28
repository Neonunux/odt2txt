/*
 * regex.c: String and regex operations for odt2txt
 *
 * Copyright (c) 2006 Dennis Stosberg <dennis@stosberg.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation
 */

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"

#define _REG_DEFAULT  0  /* Stop after first match, to be removed */
#define _REG_GLOBAL   1  /* Find all matches of regexp */
#define _REG_EXEC     2  /* subst is a function pointer */

/*
 * Deletes match(es) of regex from *buf.
 *
 * Returns the number of matches that were deleted.
 */
int regex_rm(STRBUF *buf,
	     const char *regex, int regopt);

/*
 * Replaces match(es) of regex from *buf with subst.
 */
int regex_subst(STRBUF *buf,
		const char *regex, int regopt,
		const void *subst);

/*
 * Returns a pointer to a new string with two lines. The first line
 * contains str, the second line contains strlen(str) copies of
 * linechar.
 */
char *underline(char linechar, const char *str);

/*
 * Wrappers around underline, to be used as argument to regex_subst
 * when regopt is _REG_EXEC.
 *
 * They replace the match in buf with underline('=',match) or
 * underline('-',match) respectively.
 */
char *h1(const char *buf, regmatch_t matches[], size_t nmatch, size_t off);
char *h2(const char *buf, regmatch_t matches[], size_t nmatch, size_t off);

/*
 * Prints the contents of buf to stdout, wrapped to a maximal line
 * width of width characters.
 */
void output(STRBUF *buf, int width);
