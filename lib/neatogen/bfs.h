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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/defs.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

    typedef struct {
	int *data;
	int queueSize;
	int end;
	int start;
    } Queue;

INTERNAL void mkQueue(Queue *, int);
INTERNAL void freeQueue(Queue *);
INTERNAL void initQueue(Queue *, int startVertex);
INTERNAL bool deQueue(Queue *, int *);
INTERNAL bool enQueue(Queue *, int);

INTERNAL void bfs(int, vtx_data*, int, DistType*);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
