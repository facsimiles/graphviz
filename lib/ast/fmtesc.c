/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * Glenn Fowler
 * AT&T Research
 *
 * return string with expanded escape chars
 */

#include <ast/ast.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <util/agxbuf.h>
#include <util/gv_ctype.h>

/// quote string as with qb...qe
char *fmtquote(const char *as, const char *qb, const char *qe) {
    const size_t n = strlen(as);
    const unsigned char *s = (const unsigned char *) as;
    const unsigned char *e = s + n;
    bool escaped = false;
    bool spaced = false;
    bool shell = false;

    agxbuf b = {0};
    if (qb) {
	if (qb[0] == '$' && qb[1] == '\'' && qb[2] == 0)
	    shell = true;
	agxbput(&b, qb);
    }

    char last = '\0';
#define PUT(ch)                                                                \
  do {                                                                         \
    last = (ch);                                                               \
    agxbputc(&b, (ch));                                                        \
  } while (0)

    while (s < e) {
	    int c = *s++;
	    if (gv_iscntrl(c) || !gv_isprint(c) || c == '\\') {
		escaped = true;
		PUT('\\');
		switch (c) {
		case CC_bel:
		    c = 'a';
		    break;
		case '\b':
		    c = 'b';
		    break;
		case '\f':
		    c = 'f';
		    break;
		case '\n':
		    c = 'n';
		    break;
		case '\r':
		    c = 'r';
		    break;
		case '\t':
		    c = 't';
		    break;
		case CC_vt:
		    c = 'v';
		    break;
		case CC_esc:
		    c = 'E';
		    break;
		case '\\':
		    break;
		default:
		    PUT((char)('0' + ((c >> 6) & 07)));
		    PUT((char)('0' + ((c >> 3) & 07)));
		    c = '0' + (c & 07);
		    break;
		}
	    } else if (qe && strchr(qe, c)) {
		escaped = true;
		PUT('\\');
	    } else if (!spaced &&
		       !escaped &&
		       (gv_isspace(c) ||
			(shell &&
			 (strchr("\";~&|()<>[]*?", c) ||
			  (c == '#' && (last == '\0' || gv_isspace(last))
			  )
			 )
			)
		       )
		)
		spaced = true;
	    PUT((char)c);
    }
#undef PUT
    if (qb) {
	if (!escaped) {
	    const size_t move_by = (size_t)(shell + !spaced);
	    if (move_by > 0) {
		char *content = agxbdisown(&b);
		agxbput(&b, content + move_by);
		free(content);
	    }
	}
	if (qe && (escaped || spaced))
	    agxbput(&b, qe);
    }
    return agxbdisown(&b);
}

/*
 * escape the usual suspects and quote chars in qs
 */

char *fmtesq(const char *as, const char *qs)
{
  return fmtquote(as, NULL, qs);
}

/*
 * escape the usual suspects
 */

char *fmtesc(const char *as)
{
  return fmtquote(as, NULL, NULL);
}
