/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/hedges.h>
#include <stdbool.h>

/// priority queue heap
typedef struct pq pq_t;

pq_t *PQinitialize(void);
void PQcleanup(pq_t *pq);
Halfedge *PQextractmin(pq_t *pq);
Point PQ_min(pq_t *pq);
bool PQempty(const pq_t *pq);
void PQdelete(pq_t *pq, Halfedge *);
void PQinsert(pq_t *pq, Halfedge *, Site *, double);

#ifdef __cplusplus
}
#endif
