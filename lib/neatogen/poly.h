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

#include <neatogen/geometry.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

    typedef struct {
	Point origin;
	Point corner;
	int nverts;
	Point *verts;
	int kind;
    } Poly;

INTERNAL void polyFree(void);
INTERNAL int polyOverlap(Point, Poly *, Point, Poly *);
INTERNAL int makePoly(Poly *, Agnode_t *, double, double);
INTERNAL int makeAddPoly(Poly *, Agnode_t *, double, double);
INTERNAL void breakPoly(Poly *);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
