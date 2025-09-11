/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <pathplan/pathgeom.h>
#include "Plegal_arrangement.h"
#include <stdio.h>
#include <stdlib.h>
#include "simple.h"
#include <util/alloc.h>
#include <util/list.h>
#include <util/prisize_t.h>

int Plegal_arrangement(Ppoly_t **polys, size_t n_polys) {

    int rv;

    struct data input;
    intersections_t ilist = {0};

    struct polygon *polygon_list = gv_calloc(n_polys, sizeof(struct polygon));

    size_t nverts = 0;
    for (size_t i = 0; i < n_polys; i++)
	nverts += polys[i]->pn;

    struct vertex *vertex_list = gv_calloc(nverts, sizeof(struct vertex));

    for (size_t i = 0, vno = 0; i < n_polys; i++) {
	polygon_list[i].start = &vertex_list[vno];
	for (size_t j = 0; j < polys[i]->pn; j++) {
	    vertex_list[vno].pos.x = polys[i]->ps[j].x;
	    vertex_list[vno].pos.y = polys[i]->ps[j].y;
	    vertex_list[vno].poly = &polygon_list[i];
	    vno++;
	}
	polygon_list[i].finish = &vertex_list[vno - 1];
    }

    input.nvertices = nverts;

    find_ints(vertex_list, &input, &ilist);


#define EQ_PT(v,w) (((v).x == (w).x) && ((v).y == (w).y))
    rv = 1;
    {
	struct position vft, vsd, avft, avsd;
	for (size_t i = 0; i < LIST_SIZE(&ilist); i++) {
	    struct intersection inter = LIST_GET(&ilist, i);
	    vft = inter.firstv->pos;
	    avft = after(inter.firstv)->pos;
	    vsd = inter.secondv->pos;
	    avsd = after(inter.secondv)->pos;
	    if ((vft.x != avft.x && vsd.x != avsd.x) ||
		(vft.x == avft.x && !EQ_PT(vft, inter) && !EQ_PT(avft, inter)) ||
		(vsd.x == avsd.x && !EQ_PT(vsd, inter) && !EQ_PT(avsd, inter))) {
		rv = 0;
		fprintf(stderr, "\nintersection %" PRISIZE_T " at %.3f %.3f\n",
			i, inter.x, inter.y);
		fprintf(stderr, "seg#1 : (%.3f, %.3f) (%.3f, %.3f)\n",
			inter.firstv->pos.x
			, inter.firstv->pos.y
			, after(inter.firstv)->pos.x
			, after(inter.firstv)->pos.y);
		fprintf(stderr, "seg#2 : (%.3f, %.3f) (%.3f, %.3f)\n",
			inter.secondv->pos.x
			, inter.secondv->pos.y
			, after(inter.secondv)->pos.x
			, after(inter.secondv)->pos.y);
	    }
	}
    }
    free(polygon_list);
    free(vertex_list);
    LIST_FREE(&ilist);
    return rv;
}
