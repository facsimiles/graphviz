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
#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/geometry.h>

// sites are also used as vertices on line segments
typedef struct Site {
  Point coord;
  size_t sitenbr;
  unsigned refcnt;
} Site;

extern int siteidx;
extern Site *bottomsite;

PRIVATE void siteinit(void);
PRIVATE Site *getsite(void);
PRIVATE double ngdist(Site *, Site *); /* Distance between two sites */
PRIVATE void deref(Site *);            /* Increment refcnt of site  */
PRIVATE void ref(Site *);              /* Decrement refcnt of site  */
PRIVATE void makevertex(Site *);       /* Transform a site into a vertex */

#ifdef __cplusplus
}
#endif
