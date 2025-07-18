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

#include <neatogen/defs.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

INTERNAL void fill_neighbors_vec_unweighted(vtx_data *, int vtx,
					      int *vtx_vec);
INTERNAL size_t common_neighbors(vtx_data *, int u, int *);
INTERNAL void empty_neighbors_vec(vtx_data * graph, int vtx,
				    int *vtx_vec);
INTERNAL DistType **compute_apsp(vtx_data *, int);
INTERNAL DistType **compute_apsp_artificial_weights(vtx_data *, int);
INTERNAL double distance_kD(double **, int, int, int);
INTERNAL void quicksort_place(double *, int *, int);
INTERNAL void quicksort_placef(float *, int *, int, int);
INTERNAL void compute_new_weights(vtx_data * graph, int n);
INTERNAL void restore_old_weights(vtx_data * graph, int n,
				    float *old_weights);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
