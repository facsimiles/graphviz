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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "simple.h"
#include <util/alloc.h>
#include <util/gv_math.h>
#include <util/list.h>
#include <util/prisize_t.h>

static bool eq_pt(const struct position v, const struct intersection w) {
  return is_exactly_equal(v.x, w.x) && is_exactly_equal(v.y, w.y);
}

int Plegal_arrangement(Ppoly_t **polys, size_t n_polys) {

    int rv;
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

    find_ints(vertex_list, nverts, &ilist);

    rv = 1;
    {
	for (size_t i = 0; i < LIST_SIZE(&ilist); i++) {
	    struct intersection inter = LIST_GET(&ilist, i);
	    const struct position vft = inter.firstv->pos;
	    const struct position avft = after(inter.firstv)->pos;
	    const struct position vsd = inter.secondv->pos;
	    const struct position avsd = after(inter.secondv)->pos;
	    if ((!is_exactly_equal(vft.x, avft.x) && !is_exactly_equal(vsd.x, avsd.x)) ||
		(is_exactly_equal(vft.x, avft.x) && !eq_pt(vft, inter) && !eq_pt(avft, inter)) ||
		(is_exactly_equal(vsd.x, avsd.x) && !eq_pt(vsd, inter) && !eq_pt(avsd, inter))) {
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
