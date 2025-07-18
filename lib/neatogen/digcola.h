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

#include <neatogen/defs.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

#ifdef DIGCOLA
INTERNAL int compute_y_coords(vtx_data*, int, double*, int);
INTERNAL int compute_hierarchy(vtx_data*, int, double, double, 
                         double*, int**, int**, int*); 
INTERNAL int IMDS_given_dim(vtx_data*, int, double*, double*, double);
INTERNAL int stress_majorization_with_hierarchy(vtx_data*, int, double**, 
                                              node_t**, int, int, int, int, double);
#ifdef IPSEPCOLA
typedef struct ipsep_options {
    int diredges;       /* 1=generate directed edge constraints */
                        /* 2=generate directed hierarchy level constraints (DiG-CoLa) */
    double edge_gap;    /* amount to force vertical separation of */
                        /* start/end nodes */
    int noverlap;       /* 1=generate non-overlap constraints */
                        /* 2=remove overlaps after layout */
    pointf gap;         /* hor and vert gap to enforce when removing overlap*/
    pointf* nsize;      /* node widths and heights */
    cluster_data clusters;
                        /* list of node indices for each cluster */
} ipsep_options;

 /* stress majorization, for Constraint Layout */
INTERNAL int stress_majorization_cola(vtx_data*, int, double**, node_t**, int, int, int, ipsep_options*);
#endif
#endif

#undef INTERNAL

#ifdef __cplusplus
}
#endif
