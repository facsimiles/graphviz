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

#include <neatogen/hedges.h>
#include <stdbool.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

/// priority queue heap
typedef struct pq pq_t;

INTERNAL pq_t *PQinitialize(void);
INTERNAL void PQcleanup(pq_t *pq);
INTERNAL Halfedge *PQextractmin(pq_t *pq);
INTERNAL Point PQ_min(pq_t *pq);
INTERNAL bool PQempty(const pq_t *pq);
INTERNAL void PQdelete(pq_t *pq, Halfedge *);
INTERNAL void PQinsert(pq_t *pq, Halfedge *, Site *, double);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
