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

/*	Set a new discipline for a stream.
**
**	Written by Kiem-Phong Vo
*/

Sfdisc_t *sfdisc(Sfio_t * f, Sfdisc_t * disc)
{
    Sfdisc_t *d, *rdisc;
    Sfread_f oreadf;
    Sfwrite_f owritef;
    Sfseek_f oseekf;
    ssize_t n;

    SFMTXSTART(f, NULL);

    if ((f->flags & SF_READ) && f->proc && (f->mode & SF_WRITE)) {	/* make sure in read mode to check for read-ahead data */
	if (_sfmode(f, SF_READ, 0) < 0)
	    SFMTXRETURN(f, NULL);
    } else if ((f->mode & SF_RDWR) != f->mode && _sfmode(f, 0, 0) < 0)
	SFMTXRETURN(f, NULL);

    SFLOCK(f, 0);
    rdisc = NULL;

    /* synchronize before switching to a new discipline */
    if (!(f->flags & SF_STRING)) {
	if (((f->mode & SF_WRITE) && f->next > f->data) ||
	    (f->mode & SF_READ) || f->disc == _Sfudisc)
	    (void) SFSYNC(f);

	if (((f->mode & SF_WRITE) && (n = f->next - f->data) > 0) ||
	    ((f->mode & SF_READ) && f->extent < 0
	     && (n = f->endb - f->next) > 0)) {
	    Sfexcept_f exceptf;
	    int rv = 0;

	    exceptf = disc ? disc->exceptf :
		f->disc ? f->disc->exceptf : NULL;

	    /* check with application for course of action */
	    if (exceptf) {
		SFOPEN(f, 0);
		rv = (*exceptf) (f, SF_DBUFFER, &n, disc ? disc : f->disc);
		SFLOCK(f, 0);
	    }

	    /* can't switch discipline at this time */
	    if (rv <= 0)
		goto done;
	}
    }

    /* save old readf, writef, and seekf to see if stream need reinit */
#define GETDISCF(func,iof,type) \
	{ for(d = f->disc; d && !d->iof; d = d->disc) \
	      ; \
	  func = d ? d->iof : NULL; \
	}
    GETDISCF(oreadf, readf, Sfread_f);
    GETDISCF(owritef, writef, Sfwrite_f);
    GETDISCF(oseekf, seekf, Sfseek_f);

    if (disc == SF_POPDISC) {	/* popping, warn the being popped discipline */
	if (!(d = f->disc))
	    goto done;
	disc = d->disc;
	if (d->exceptf) {
	    SFOPEN(f, 0);
	    if ((*(d->exceptf)) (f, SF_DPOP, (void *) disc, d) < 0)
		goto done;
	    SFLOCK(f, 0);
	}
	f->disc = disc;
	rdisc = d;
    } else {			/* pushing, warn being pushed discipline */
	do {			/* loop to handle the case where d may pop itself */
	    d = f->disc;
	    if (d && d->exceptf) {
		SFOPEN(f, 0);
		if ((*(d->exceptf)) (f, SF_DPUSH, (void *) disc, d) < 0)
		    goto done;
		SFLOCK(f, 0);
	    }
	} while (d != f->disc);

	/* make sure we are not creating an infinite loop */
	for (; d; d = d->disc)
	    if (d == disc)
		goto done;

	/* set new disc */
	disc->disc = f->disc;
	f->disc = disc;
	rdisc = disc;
    }

    if (!(f->flags & SF_STRING)) {	/* this stream may have to be reinitialized */
	int reinit = 0;
#define DISCF(dst,iof,type)	(dst ? dst->iof : NULL)
#define REINIT(oiof,iof,type) \
		if(!reinit) \
		{	for(d = f->disc; d && !d->iof; d = d->disc) \
				; \
			if(DISCF(d,iof,type) != oiof) \
				reinit = 1; \
		}

	REINIT(oreadf, readf, Sfread_f);
	REINIT(owritef, writef, Sfwrite_f);
	REINIT(oseekf, seekf, Sfseek_f);

	if (reinit) {
	    SETLOCAL(f);
	    f->bits &= (unsigned short)~SF_NULL;	/* turn off /dev/null handling */
	    if ((f->bits & SF_MMAP) || (f->mode & SF_INIT))
		sfsetbuf(f, NULL, SF_UNBOUND);
	    else if (f->data == f->tiny)
		sfsetbuf(f, NULL, 0);
	    else {
		unsigned short flags = f->flags;
		sfsetbuf(f, (void *) f->data, f->size);
		f->flags |= (flags & SF_MALLOC);
	    }
	}
    }

  done:
    SFOPEN(f, 0);
    SFMTXRETURN(f, rdisc);
}
