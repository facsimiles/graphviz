/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cgraph.h"

    typedef Dt_t queue;

    extern queue *mkQ(Dtmethod_t *);
    extern void push(queue *, void *);
    extern void *pop(queue *, int remove);
    extern void freeQ(queue *);

/* pseudo-functions:
extern queue* mkStack();
extern queue* mkQueue();
extern void* pull(queue*);
extern void* head(queue*);
 */

#define mkStack()  mkQ(Dtstack)
#define mkQueue()  mkQ(Dtqueue)
#define pull(q)  (pop(q,1))
#define head(q)  (pop(q,0))

#ifdef __cplusplus
}
#endif
