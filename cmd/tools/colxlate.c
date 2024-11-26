/**
 * @file
 * @brief function colorxlate (one of many) for gvcolor.c
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <cgraph/gv_ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <util/agxbuf.h>
typedef struct {
    char *name;
    unsigned char h, s, b;
} hsbcolor_t;

#include "colortbl.h"
#include "colorxlate.h"

static void canoncolor(const char *orig, agxbuf *out) {
    char c;
    while ((c = *orig++)) {
	if (!gv_isalnum(c))
	    continue;
	agxbputc(out, (char)tolower(c));
    }
}

static int colorcmpf(const void *a0, const void *a1)
{
    const hsbcolor_t *p1 = a1;
    return strcmp(a0, p1->name);
}

void colorxlate(char *str, agxbuf *buf) {
    static hsbcolor_t *last;
    agxbuf canon_buf = {0};
    const char *canon = NULL;

    if (last == NULL || strcmp(last->name, str)) {
	canoncolor(str, &canon_buf);
	canon = agxbuse(&canon_buf);
	last = bsearch(canon, color_lib, sizeof(color_lib) / sizeof(hsbcolor_t),
	               sizeof(color_lib[0]), colorcmpf);
    }
    if (last == NULL) {
	if (!gv_isdigit(canon[0])) {
	    fprintf(stderr, "warning: %s is not a known color\n", str);
	    agxbput(buf, str);
	} else
	    for (const char *p = str; *p != '\0'; ++p)
		agxbputc(buf, *p == ',' ? ' ' : *p);
    } else
	agxbprint(buf, "%.3f %.3f %.3f", ((double) last->h) / 255,
		((double) last->s) / 255, ((double) last->b) / 255);
    agxbfree(&canon_buf);
}
