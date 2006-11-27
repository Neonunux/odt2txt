/*
 * stringops.c: String and regex operations for odt2txt
 *
 * Copyright (c) 2006 Dennis Stosberg <dennis@stosberg.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation
 */

#include "mem.h"
#include "regex.h"

#define BUF_SZ 4096

static char *headline(char line, const char *buf, regmatch_t matches[],
		      size_t nmatch, size_t off);
static size_t charlen_utf8(const char *s);

void print_regexp_err(int reg_errno, const regex_t *rx)
{
	char *buf = ymalloc(BUF_SZ);

	regerror(reg_errno, rx, buf, BUF_SZ);
	fprintf(stderr, "%s\n", buf);

	yfree(buf);
}

int regex_subst(STRBUF *buf,
		const char *regex, int regopt,
		const void *subst)
{
	int r;
	const char *bufp;
	size_t off = 0;
	const int i = 0;
	int match_count = 0;

	regex_t rx;
	const size_t nmatches = 10;
	regmatch_t matches[nmatches];

	r = regcomp(&rx, regex, REG_EXTENDED);
	if (r) {
		print_regexp_err(r, &rx);
		exit(EXIT_FAILURE);
	}

	do {
		if (off > strbuf_len(buf))
			break;

		bufp = strbuf_get(buf) + off;

		if (0 != regexec(&rx, bufp, nmatches, matches, 0))
			break;

		if (matches[i].rm_so != -1) {
			char *s;
			int subst_len;

			if (regopt & _REG_EXEC) {
				s = (*(char *(*)
				       (const char *buf, regmatch_t matches[],
					size_t nmatch, size_t off))subst)
					(strbuf_get(buf), matches, nmatches, off);
			} else
				s = (char*)subst;

			subst_len = strbuf_subst(buf,
						 matches[i].rm_so + off,
						 matches[i].rm_eo + off,
						 s);
			match_count++;

			if (regopt & _REG_EXEC)
				yfree(s);

			off += matches[i].rm_so;
			if (subst_len >= 0)
				off += subst_len + 1;
		}
	} while (regopt & _REG_GLOBAL);

	regfree(&rx);
	return match_count;
}

int regex_rm(STRBUF *buf,
	     const char *regex, int regopt)
{
	return regex_subst(buf, regex, regopt, "");
}

char *underline(char linechar, const char *lenstr)
{
	int i;
	char *tmp;
	STRBUF *line;
	size_t charlen = charlen_utf8(lenstr);

	if (lenstr[0] == '\0') {
		tmp = ymalloc(1);
		tmp[0] = '\0';
		return tmp;
	}

	line = strbuf_new();
	strbuf_append(line, lenstr);
	strbuf_append(line, "\n");

	tmp = ymalloc(charlen);
	for (i = 0; i < charlen; i++) {
		tmp[i] = linechar;
	}
	strbuf_append_n(line, tmp, charlen);
	yfree(tmp);

	strbuf_append(line, "\n\n");
	return strbuf_spit(line);
}

static char *headline(char line, const char *buf, regmatch_t matches[],
		      size_t nmatch, size_t off)
{
	const int i = 1;
	char *result;
	size_t len;
	char *match;

	len = matches[i].rm_eo - matches[i].rm_so;
	match = ymalloc(len + 1);

	memcpy(match, buf + matches[i].rm_so + off, len);
	match[len] = '\0' ;

	result = underline(line, match);

	yfree(match);
	return result;
}

char *h1(const char *buf, regmatch_t matches[], size_t nmatch, size_t off)
{
	return headline('=', buf, matches, nmatch, off);
}

char *h2(const char *buf, regmatch_t matches[], size_t nmatch, size_t off)
{
	return headline('-', buf, matches, nmatch, off);
}

static size_t charlen_utf8(const char *s)
{
	size_t count = 0;
	unsigned char *t = (unsigned char*) s;
	while (*t != '\0') {
		if (*t > 0x80)
			t++;
		if (*t > 0xDF)
			t++;
		if (*t > 0xF0)
			t++;
		count++;
		t++;
	}
	return count;
}

void output(STRBUF *buf, int width)
{
	/* FIXME: This function should take multibyte utf8-encoded
	   characters into account for the length calculation. */

	const char *lf = "\n  ";
	const size_t lflen = strlen(lf);
	const char *bufp;
	const char *last;
	const char *lastspace = 0;
	int linelen = 0;

	bufp = strbuf_get(buf);
	last = bufp;

	fwrite(lf, lflen, 1, stdout);
	while (*bufp) {
		if (*bufp == ' ')
			lastspace = bufp;
		else if (*bufp == '\n') {
			while(*last == ' ')
				last++;

			fwrite(last, (size_t)(bufp - last), 1, stdout);
			fwrite(lf, lflen, 1, stdout);
 			last = bufp + 1;
			linelen = 0;
		}

		if (linelen >= width) {
			while(*last == ' ')
				last++;

			fwrite(last, (size_t)(lastspace - last), 1, stdout);
			fwrite(lf, lflen, 1, stdout);
			last = lastspace;
			linelen = 0;
		}

		bufp++;
		linelen++;
	}
	fputs("\n", stdout);
}

