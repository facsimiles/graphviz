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

#include <neatogen/sparsegraph.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

typedef struct {
    int  nedges; /* no. of edges in triangulation */
    int* edges;  /* 2*nsegs indices of points */
    int  nfaces; /* no. of faces in triangulation */
    int* faces;  /* 3*nfaces indices of points */ 
    int* neigh;  /* 3*nfaces indices of neighbor triangles */ 
} surface_t;

INTERNAL int *delaunay_tri (double *x, double *y, int n, int* nedges);

INTERNAL int *get_triangles (double *x, int n, int* ntris);

INTERNAL v_data *UG_graph(double *x, double *y, int n);

INTERNAL surface_t* mkSurface (double *x, double *y, int n, int* segs, int nsegs);

INTERNAL void freeSurface (surface_t* s);

#undef INTERNAL
