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
#include <neatogen/sgd.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

INTERNAL void ngdijkstra(int, vtx_data *, int, DistType *);
INTERNAL void dijkstra_f(int, vtx_data *, int, float *);
INTERNAL int dijkstra_sgd(graph_sgd *, int, term_sgd *);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
