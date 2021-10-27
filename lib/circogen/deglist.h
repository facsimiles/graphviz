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

/* List of nodes sorted by increasing degree */

#include  <render.h>

    typedef Dt_t deglist_t;

    extern deglist_t *mkDeglist(void);
    extern void freeDeglist(deglist_t * list);
    extern void insertDeglist(deglist_t * list, Agnode_t * n);
    extern void removeDeglist(deglist_t * list, Agnode_t * n);
    extern Agnode_t *firstDeglist(deglist_t *);

#ifdef DEBUG
    extern void printDeglist(deglist_t *);
#endif

#ifdef __cplusplus
}
#endif
