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

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/site.h>
#include <neatogen/edges.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

    typedef struct Halfedge {
	struct Halfedge *ELleft, *ELright;
	Edge *ELedge;
	int ELrefcnt;
	char ELpm;
	Site *vertex;
	double ystar;
	struct Halfedge *PQnext;
    } Halfedge;

    extern Halfedge *ELleftend, *ELrightend;

INTERNAL void ELinitialize(void);
INTERNAL void ELcleanup(void);
INTERNAL int right_of(Halfedge *, Point *);
INTERNAL Site *hintersect(Halfedge *, Halfedge *);
INTERNAL Halfedge *HEcreate(Edge *, char);
INTERNAL void ELinsert(Halfedge *, Halfedge *);
INTERNAL Halfedge *ELleftbnd(Point *);
INTERNAL void ELdelete(Halfedge *);
INTERNAL Halfedge *ELleft(Halfedge *), *ELright(Halfedge *);
INTERNAL Site *leftreg(Halfedge *), *rightreg(Halfedge *);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
