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
#define PRIVATE __attribute__((visibility("hidden")))
#else
#define PRIVATE /* nothing */
#endif

    typedef struct {
	int *data;
	int queueSize;
	int end;
	int start;
    } Queue;

PRIVATE void mkQueue(Queue *, int);
PRIVATE void freeQueue(Queue *);
PRIVATE void initQueue(Queue *, int startVertex);
PRIVATE bool deQueue(Queue *, int *);
PRIVATE bool enQueue(Queue *, int);

PRIVATE void bfs(int, vtx_data*, int, DistType*);

#undef PRIVATE

#ifdef __cplusplus
}
#endif
