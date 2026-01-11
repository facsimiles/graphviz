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

#include "simple.h"
#include <stdio.h>
#include <stdlib.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/list.h>

static int gt(const void *a, const void *b);

void find_ints(struct vertex vertex_list[], size_t nvertices,
               intersections_t *ilist) {
    int k;
    LIST(struct active_edge *) all = {.dtor = LIST_DTOR_FREE};
    struct active_edge *tempa;
    struct vertex *pt1, *pt2, *templ;

    struct vertex **pvertex = gv_calloc(nvertices, sizeof(struct vertex*));

    for (size_t i = 0; i < nvertices; i++)
	pvertex[i] = vertex_list + i;

/* sort vertices by x coordinate	*/
    qsort(pvertex, nvertices, sizeof(struct vertex *), gt);

/* walk through the vertices in order of increasing x coordinate	*/
    for (size_t i = 0; i < nvertices; i++) {
	pt1 = pvertex[i];
	templ = pt2 = prior(pvertex[i]);
	for (k = 0; k < 2; k++) {	/* each vertex has 2 edges */
	    switch (gt(&pt1, &pt2)) {

	    case -1:		/* forward edge, test and insert      */
		for (size_t j = 0; j < LIST_SIZE(&all); ++j) {
		    tempa = LIST_GET(&all, j);
		    find_intersection(tempa->name, templ, ilist); // test
		}

		struct active_edge *new = gv_alloc(sizeof(struct active_edge));
		LIST_APPEND(&all, new);

		new->name = templ;
		templ->active = new;
		break;

	    case 1:		/* backward edge, delete        */

		if ((tempa = templ->active) == NULL) {
		    fprintf(stderr,
			    "\n***ERROR***\n trying to delete a non line\n");
		    graphviz_exit(1);
		}
		LIST_REMOVE(&all, tempa);
		templ->active = NULL;
		break;

	    default:
		break;
	    }

	    pt2 = after(pvertex[i]);
	    templ = pvertex[i];	/*second neighbor */
	}
    }
    LIST_FREE(&all);
    free(pvertex);
}

static int gt(const void *a, const void *b) {
    const struct vertex *const *i = a;
    const struct vertex *const *j = b;
    if ((*i)->pos.x > (*j)->pos.x) {
      return 1;
    }
    if ((*i)->pos.x < (*j)->pos.x) {
      return -1;
    }
    if ((*i)->pos.y > (*j)->pos.y) {
      return 1;
    }
    if ((*i)->pos.y < (*j)->pos.y) {
      return -1;
    }
    return 0;
}
