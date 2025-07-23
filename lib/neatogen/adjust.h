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

#include "geom.h"
#include <sparse/SparseMatrix.h>
#include <stdbool.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

#define DFLT_MARGIN     4   /* 4 points */

typedef enum {
    AM_NONE, AM_VOR,
    AM_SCALE, AM_NSCALE, AM_SCALEXY, AM_PUSH, AM_PUSHPULL,
    AM_ORTHO, AM_ORTHO_YX, AM_ORTHOXY, AM_ORTHOYX,
    AM_PORTHO, AM_PORTHO_YX, AM_PORTHOXY, AM_PORTHOYX, AM_COMPRESS,
    AM_VPSC, AM_IPSEP, AM_PRISM
} adjust_mode;

typedef struct {
    adjust_mode mode;
    char *print;
    int value;
    double scaling;
} adjust_data;

typedef struct {
    double x, y;
    bool doAdd;  /* if true, x and y are in points */
} expand_t;

INTERNAL expand_t sepFactor(graph_t * G);
INTERNAL expand_t esepFactor(graph_t * G);
INTERNAL int adjustNodes(graph_t * G);
INTERNAL int normalize(graph_t * g);
INTERNAL int removeOverlapAs(graph_t*, char*);
INTERNAL int removeOverlapWith(graph_t*, adjust_data*);
INTERNAL int cAdjust(graph_t *, int);
INTERNAL int scAdjust(graph_t *, int);
INTERNAL void graphAdjustMode(graph_t *G, adjust_data*, char* dflt);
INTERNAL double *getSizes(Agraph_t * g, pointf pad, int *n_elabels, int **elabels);
INTERNAL SparseMatrix makeMatrix(Agraph_t *g);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
