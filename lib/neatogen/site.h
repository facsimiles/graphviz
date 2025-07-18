/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/geometry.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

    /* Sites are also used as vertices on line segments */
    typedef struct Site {
	Point coord;
	size_t sitenbr;
	unsigned refcnt;
    } Site;

    extern int siteidx;
    extern Site *bottomsite;

INTERNAL void siteinit(void);
INTERNAL Site *getsite(void);
INTERNAL double ngdist(Site *, Site *);	/* Distance between two sites */
INTERNAL void deref(Site *);	/* Increment refcnt of site  */
INTERNAL void ref(Site *);	/* Decrement refcnt of site  */
INTERNAL void makevertex(Site *);	/* Transform a site into a vertex */

#undef INTERNAL

#ifdef __cplusplus
}
#endif
