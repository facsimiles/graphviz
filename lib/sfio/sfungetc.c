/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<sfio/sfhdr.h>
#include	<stddef.h>

/*	Push back one byte to a given SF_READ stream
**
**	Written by Kiem-Phong Vo.
*/
static int _uexcept(Sfio_t * f, int type, void * val,
		    Sfdisc_t * disc)
{
    NOTUSED(val);

    /* hmm! This should never happen */
    if (disc != _Sfudisc)
	return -1;

    /* close the unget stream */
    if (type != SF_CLOSING)
	(void) sfclose((*_Sfstack) (f, NULL));

    return 1;
}

/**
 * @param f push back one byte to this stream
 * @param c the value to be pushed back
 */
int sfungetc(Sfio_t * f, int c)
{
    Sfio_t *uf;

    SFMTXSTART(f, -1)

	if (c < 0 || (f->mode != SF_READ && _sfmode(f, SF_READ, 0) < 0))
	SFMTXRETURN(f, -1);
    SFLOCK(f, 0);

    /* fast handling of the typical unget */
    if (f->next > f->data && f->next[-1] == (uchar) c) {
	f->next -= 1;
	goto done;
    }

    /* make a string stream for unget characters */
    if (f->disc != _Sfudisc) {
	if (!(uf = sfnew(NULL, NULL, SF_UNBOUND, -1, SF_STRING|SF_READ))) {
	    c = -1;
	    goto done;
	}
	_Sfudisc->exceptf = _uexcept;
	sfdisc(uf, _Sfudisc);
	SFOPEN(f, 0);
	(void) sfstack(f, uf);
	SFLOCK(f, 0);
    }

    /* space for data */
    if (f->next == f->data) {
	uchar *data;
	if (f->size < 0)
	    f->size = 0;
	if (!(data = malloc(f->size + 16))) {
	    c = -1;
	    goto done;
	}
	f->flags |= SF_MALLOC;
	if (f->data)
	    memcpy(data + 16, f->data, f->size);
	f->size += 16;
	f->data = data;
	f->next = data + 16;
	f->endb = data + f->size;
    }

    *--f->next = (uchar) c;
  done:
    SFOPEN(f, 0);
    SFMTXRETURN(f, c);
}
