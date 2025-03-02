/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "smyrnadefs.h"
#include "hier.h"
#include <math.h>
#include <neatogen/delaunay.h>
#include <stdbool.h>
#include <stddef.h>
#include <util/alloc.h>

void positionAllItems(Hierarchy * hp, focus_t * fs, reposition_t * parms)
{
    int interval = 20;
    size_t counter = 0; // no. of active nodes
    double *x_coords = gv_calloc(hp->nvtxs[0], sizeof(double));
    double *y_coords = gv_calloc(hp->nvtxs[0], sizeof(double));
    int max_level = hp->nlevels - 1;	// coarsest level
    double width = parms->width;
    double height = parms->height;
    double distortion = parms->distortion;

    /* get all logical coordinates of active nodes */
    for (int i = 0; i < hp->nvtxs[max_level]; i++) {
	counter =
	    extract_active_logical_coords(hp, i, max_level, x_coords,
					  y_coords, counter);
    }

    /* distort logical coordinates in order to get uniform density
     * (equivalent to concentrating on the focus area)
     */
    if (fs->num_foci != 0) {
	rescale_layout_polar(x_coords, y_coords, fs->x_foci,
				 fs->y_foci, fs->num_foci, counter,
				 interval, width, height, distortion);
    }

    /* Update the final physical coordinates of the active nodes */
    for (int count = 0, i = 0; i < hp->nvtxs[max_level]; i++) {
	count =
	    set_active_physical_coords(hp, i, max_level, x_coords,
				       y_coords, count);
    }

    free(x_coords);
    free(y_coords);
}

#ifdef DEBUG
static void dumpG(int nn, v_data * graph)
{
    int i, j;
    for (i = 0; i < nn; i++) {
	fprintf(stderr, "[%d]", i);
	for (j = 1; j < graph->nedges; j++)
	    fprintf(stderr, " %d", graph->edges[j]);
	fprintf(stderr, "\n");
	graph++;
    }
}

static void dumpEG(int nn, ex_vtx_data * graph)
{
    int i, j;
    for (i = 0; i < nn; i++) {
	fprintf(stderr, "[%d](%d,%d,%d)(%f,%f)", i, graph->size,
		graph->active_level, graph->globalIndex, graph->x_coord,
		graph->y_coord);
	for (j = 1; j < graph->nedges; j++)
	    fprintf(stderr, " %d", graph->edges[j]);
	fprintf(stderr, "\n");
	graph++;
    }
}

static void dumpHier(Hierarchy * hier)
{
    int i;

    for (i = 0; i < hier->nlevels; i++) {
	fprintf(stderr, "level [%d] %d %d \n", i, hier->nvtxs[i],
		hier->nedges[i]);
	fprintf(stderr, "graph\n");
	dumpG(hier->nvtxs[i], hier->graphs[0]);
	fprintf(stderr, "geom_graph\n");
	dumpEG(hier->nvtxs[i], hier->geom_graphs[0]);
    }
}

#endif

Hierarchy *makeHier(int nn, int ne, v_data *graph, double *x_coords,
                    double *y_coords, bool dist2_limit) {
    v_data *delaunay;
    ex_vtx_data *geom_graph;
    int ngeom_edges;
    Hierarchy *hp;
    int i;

    delaunay = UG_graph(x_coords, y_coords, nn);

    ngeom_edges =
	init_ex_graph(delaunay, graph, nn, x_coords, y_coords,
		      &geom_graph);
    free(delaunay[0].edges);
    free(delaunay);

    hp = create_hierarchy(graph, nn, ne, geom_graph, ngeom_edges, dist2_limit);
    free(geom_graph[0].edges);
    free(geom_graph);

    init_active_level(hp, 0);
    geom_graph = hp->geom_graphs[0];
    for (i = 0; i < hp->nvtxs[0]; i++) {
	geom_graph[i].physical_x_coord = (float) x_coords[i];
	geom_graph[i].physical_y_coord = (float) y_coords[i];
    }

    return hp;
}

focus_t *initFocus(int ncnt)
{
    focus_t *fs = gv_alloc(sizeof(focus_t));
    fs->num_foci = 0;
    fs->foci_nodes = gv_calloc(ncnt, sizeof(int));
    fs->x_foci = gv_calloc(ncnt, sizeof(double));
    fs->y_foci = gv_calloc(ncnt, sizeof(double));
    return fs;
}
