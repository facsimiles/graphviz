/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/hedges.h>
#include <stdbool.h>

/// priority queue heap
typedef struct pq pq_t;

PRIVATE pq_t *PQinitialize(void);
PRIVATE void PQcleanup(pq_t *pq);
PRIVATE Halfedge *PQextractmin(pq_t *pq);
PRIVATE Point PQ_min(pq_t *pq);
PRIVATE bool PQempty(const pq_t *pq);
PRIVATE void PQdelete(pq_t *pq, Halfedge *);
PRIVATE void PQinsert(pq_t *pq, Halfedge *, Site *, double);

#ifdef __cplusplus
}
#endif
