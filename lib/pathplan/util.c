/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <pathplan/pathutil.h>
#include <util/alloc.h>
#include <util/list.h>

void freePath(Ppolyline_t* p)
{
    free(p->ps);
    free(p);
}

int Ppolybarriers(Ppoly_t ** polys, int npolys, Pedge_t ** barriers,
		  int *n_barriers)
{
    int i;

    LIST(Pedge_t) bar = {0};

    for (i = 0; i < npolys; i++) {
	const Ppoly_t pp = *polys[i];
	for (size_t j = 0; j < pp.pn; j++) {
	    size_t k = j + 1;
	    if (k >= pp.pn)
		k = 0;
	    LIST_APPEND(&bar, ((Pedge_t){.a = pp.ps[j], .b = pp.ps[k]}));
	}
    }
    size_t n;
    LIST_DETACH(&bar, barriers, &n);
    assert(n <= INT_MAX);
    *n_barriers = (int)n;
    return 1;
}

/* make_polyline:
 */
void
make_polyline(Ppolyline_t line, Ppolyline_t* sline)
{
    static size_t isz = 0;
    static Ppoint_t* ispline = 0;
    const size_t npts = 4 + 3 * (line.pn - 2);

    if (npts > isz) {
	ispline = gv_recalloc(ispline, isz, npts, sizeof(Ppoint_t));
	isz = npts;
    }

    size_t j = 0;
    size_t i = 0;
    ispline[j+1] = ispline[j] = line.ps[i];
    j += 2;
    i++;
    for (; i + 1 < line.pn; i++) {
	ispline[j+2] = ispline[j+1] = ispline[j] = line.ps[i];
	j += 3;
    }
    ispline[j+1] = ispline[j] = line.ps[i];

    sline->pn = npts;
    sline->ps = ispline;
}

/**
 * @dir lib/pathplan
 * @brief finds and smooths shortest paths, API pathplan.h
 *
 * [man 3 pathplan](https://graphviz.org/pdf/pathplan.3.pdf)
 *
 */
