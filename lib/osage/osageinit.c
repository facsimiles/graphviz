/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/


/* FIX: handle cluster labels */
/*
 * Written by Emden R. Gansner
 */

#include "config.h"

#include    <assert.h>
#include    <common/geomprocs.h>
#include    <common/render.h>
#include    <common/utils.h>
#include    <float.h>
#include    <limits.h>
#include    <osage/osage.h>
#include    <neatogen/neatoprocs.h>
#include    <pack/pack.h>
#include    <stdbool.h>
#include    <stddef.h>
#include    <util/alloc.h>
#include    <util/debug.h>
#include    <util/list.h>
#include    <util/startswith.h>

#define DFLT_SZ  18
#define PARENT(n) ((Agraph_t*)ND_alg(n))

static void
indent (int i)
{
    for (; i > 0; i--)
	fputs ("  ", stderr);
}

typedef LIST(Agraph_t *) clist_t;

static void cluster_init_graph(graph_t * g)
{
    Agnode_t *n;
    Agedge_t *e;

    setEdgeType (g, EDGETYPE_LINE);
    Ndim = GD_ndim(g)=2;	/* The algorithm only makes sense in 2D */

    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	neato_init_node (n);
    }
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	for (e = agfstout(g, n); e; e = agnxtout(g, e)) {
	    agbindrec(e, "Agedgeinfo_t", sizeof(Agedgeinfo_t), true);	//edge custom data
	    common_init_edge(e);
	}
    }
}

/* layout:
 */
static void
layout (Agraph_t* g, int depth)
{
    int nvs = 0;       /* no. of nodes in subclusters */
    pack_info pinfo;
    Agsym_t* cattr = NULL;
    Agsym_t* vattr = NULL;
    Agraph_t* root = g->root;

    if (Verbose > 1) {
	indent (depth);
	fprintf (stderr, "layout %s\n", agnameof(g));
    }

    /* Lay out subclusters */
    for (int i = 1; i <= GD_n_cluster(g); i++) {
        Agraph_t *const subg = GD_clust(g)[i];
	layout (subg, depth+1);
	nvs += agnnodes (subg);
    }

    const int nv = agnnodes(g);
    const int total = nv - nvs + GD_n_cluster(g);

    if (total == 0 && GD_label(g) == NULL) {
	GD_bb(g) = (boxf){.UR = {.x = DFLT_SZ, .y = DFLT_SZ}};
	return;
    }
    
    const pack_mode pmode = getPackInfo(g, l_array, DFLT_MARGIN, &pinfo);
    if (pmode < l_graph) pinfo.mode = l_graph;

        /* add user sort values if necessary */
    if (pinfo.mode == l_array && (pinfo.flags & PK_USER_VALS)) {
	cattr = agattr_text(root, AGRAPH, "sortv", 0);
	vattr = agattr_text(root, AGNODE, "sortv", 0);
	if (cattr == NULL && vattr == NULL)
	    agwarningf("Graph %s has array packing with user values but no \"sortv\" attributes are defined.",
		agnameof(g));
    }

    LIST(boxf) gs = {0};
    LIST(void *) children = {0};
    LIST(packval_t) vals = {0};
    for (int i = 1; i <= GD_n_cluster(g); i++) {
	Agraph_t *const subg = GD_clust(g)[i];
	LIST_APPEND(&gs, GD_bb(subg));
	if (cattr) {
	    LIST_APPEND(&vals, (packval_t)late_int(subg, cattr, 0, 0));
	}
	LIST_APPEND(&children, subg);
    }
  
    if (nv-nvs > 0) {
	for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
	    if (ND_alg(n)) continue;
	    ND_alg(n) = g;
	    const boxf bb = {.UR = {.x = ND_xsize(n), .y = ND_ysize(n)}};
	    LIST_APPEND(&gs, bb);
	    if (vattr) {
		LIST_APPEND(&vals, (packval_t)late_int(n, vattr, 0, 0));
	    }
	    LIST_APPEND(&children, n);
	}
    }

	/* pack rectangles */
    pinfo.vals = LIST_IS_EMPTY(&vals) ? NULL : LIST_FRONT(&vals);
    pointf *const pts = putRects(LIST_SIZE(&gs),
                                 LIST_IS_EMPTY(&gs) ? NULL : LIST_FRONT(&gs),
                                 &pinfo);

    boxf rootbb = {.LL = {DBL_MAX, DBL_MAX}, .UR = {-DBL_MAX, -DBL_MAX}};

    /* reposition children relative to GD_bb(g) */
    for (size_t i = 0; i < LIST_SIZE(&gs); ++i) {
	const pointf p = pts[i];
	boxf bb = LIST_GET(&gs, i);
	bb.LL = add_pointf(bb.LL, p);
	bb.UR = add_pointf(bb.UR, p);
	EXPANDBB(&rootbb, bb);
	if (i < (size_t)GD_n_cluster(g)) {
	    Agraph_t *const subg = LIST_GET(&children, i);
	    GD_bb(subg) = bb;
	    if (Verbose > 1) {
		indent (depth);
		fprintf (stderr, "%s : %f %f %f %f\n", agnameof(subg), bb.LL.x, bb.LL.y, bb.UR.x, bb.UR.y);
	    }
	}
	else {
	    Agnode_t *const n = LIST_GET(&children, i);
	    if (n) {
	        ND_coord(n) = mid_pointf (bb.LL, bb.UR);
	        if (Verbose > 1) {
		    indent (depth);
		    fprintf (stderr, "%s : %f %f\n", agnameof(n), ND_coord(n).x, ND_coord(n).y);
		}
	    }
	}
    }

    if (GD_label(g)) {
        double d;

        pointf pt = GD_label(g)->dimen;
	if (total == 0) {
            rootbb = (boxf){.UR = pt};
	}
        d = pt.x - (rootbb.UR.x - rootbb.LL.x);
        if (d > 0) {            /* height of label is added below */
            d /= 2;
            rootbb.LL.x -= d;
            rootbb.UR.x += d;
        }
    }

    const double margin = depth > 0 ? pinfo.margin / 2.0 : 0;
    rootbb.LL.x -= margin;
    rootbb.UR.x += margin;
    rootbb.LL.y -= margin + GD_border(g)[BOTTOM_IX].y;
    rootbb.UR.y += margin + GD_border(g)[TOP_IX].y;

    if (Verbose > 1) {
	indent (depth);
	fprintf (stderr, "%s : %f %f %f %f\n", agnameof(g), rootbb.LL.x, rootbb.LL.y, rootbb.UR.x, rootbb.UR.y);
    }

    /* Translate so that rootbb.LL is origin.
     * This makes the repositioning simpler; we just translate the clusters and nodes in g by
     * the final bb.ll of g.
     */
    for (size_t i = 0; i < LIST_SIZE(&children); ++i) {
	if (i < (size_t)GD_n_cluster(g)) {
	    Agraph_t *const subg = LIST_GET(&children, i);
	    boxf bb = GD_bb(subg);
	    bb.LL = sub_pointf(bb.LL, rootbb.LL);
	    bb.UR = sub_pointf(bb.UR, rootbb.LL);
	    GD_bb(subg) = bb;
	    if (Verbose > 1) {
		indent (depth);
		fprintf (stderr, "%s : %f %f %f %f\n", agnameof(subg), bb.LL.x, bb.LL.y, bb.UR.x, bb.UR.y);
	    }
	}
	else {
	    Agnode_t *const n = LIST_GET(&children, i);
	    if (n) {
	        ND_coord(n) = sub_pointf (ND_coord(n), rootbb.LL);
	        if (Verbose > 1) {
		    indent (depth);
		    fprintf (stderr, "%s : %f %f\n", agnameof(n), ND_coord(n).x, ND_coord(n).y);
		}
	    }
	}
    }

    rootbb = (boxf){.UR = sub_pointf(rootbb.UR, rootbb.LL)};
    GD_bb(g) = rootbb;

    if (Verbose > 1) {
	indent (depth);
	fprintf (stderr, "%s : %f %f %f %f\n", agnameof(g), rootbb.LL.x, rootbb.LL.y, rootbb.UR.x, rootbb.UR.y);
    }

    LIST_FREE(&vals);
    LIST_FREE(&gs);
    LIST_FREE(&children);
    free (pts);
}

static void
reposition (Agraph_t* g, int depth)
{
    boxf bb = GD_bb(g);

    if (Verbose > 1) {
	indent (depth);
	fprintf (stderr, "reposition %s\n", agnameof(g));
    }

    /* translate nodes in g but not in a subcluster */
    if (depth) {
        for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
            if (PARENT(n) != g)
                continue;
            ND_coord(n).x += bb.LL.x;
            ND_coord(n).y += bb.LL.y;
	    if (Verbose > 1) {
		indent (depth);
		fprintf (stderr, "%s : %f %f\n", agnameof(n), ND_coord(n).x, ND_coord(n).y);
	    }
        }
    }

    /* translate top-level clusters and recurse */
    for (int i = 1; i <= GD_n_cluster(g); i++) {
        Agraph_t *const subg = GD_clust(g)[i];
        if (depth) {
            boxf sbb = GD_bb(subg);
            sbb.LL.x += bb.LL.x;
            sbb.LL.y += bb.LL.y;
            sbb.UR.x += bb.LL.x;
            sbb.UR.y += bb.LL.y;
	    if (Verbose > 1) {
		indent (depth);
		fprintf (stderr, "%s : %f %f %f %f\n", agnameof(subg), sbb.LL.x, sbb.LL.y, sbb.UR.x, sbb.UR.y);
	    }
            GD_bb(subg) = sbb;
        }
        reposition (subg, depth+1);
    }

}

static void
mkClusters (Agraph_t* g, clist_t* pclist, Agraph_t* parent)
{
    clist_t  list = {0};
    clist_t* clist;

    if (pclist == NULL) {
        // [0] is empty. The clusters are in [1..cnt].
        LIST_APPEND(&list, NULL);
        clist = &list;
    }
    else
        clist = pclist;

    for (graph_t *subg = agfstsubg(g); subg; subg = agnxtsubg(subg)) {
        if (is_a_cluster(subg)) {
	    agbindrec(subg, "Agraphinfo_t", sizeof(Agraphinfo_t), true);
	    do_graph_label (subg);
            LIST_APPEND(clist, subg);
            mkClusters(subg, NULL, subg);
        }
        else {
            mkClusters(subg, clist, parent);
        }
    }
    if (pclist == NULL) {
        assert(LIST_SIZE(&list) - 1 <= INT_MAX);
        GD_n_cluster(g) = (int)(LIST_SIZE(&list) - 1);
        if (LIST_SIZE(&list) > 1) {
            LIST_SHRINK_TO_FIT(&list);
            LIST_DETACH(&list, &GD_clust(g), NULL);
        } else {
            LIST_FREE(&list);
        }
    }
}

void osage_layout(Agraph_t *g)
{
    cluster_init_graph(g);
    mkClusters(g, NULL, g);
    layout(g, 0);
    reposition (g, 0);

    if (GD_drawing(g)->ratio_kind) {
	Agnode_t* n;
	for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	    ND_pos(n)[0] = PS2INCH(ND_coord(n).x);
	    ND_pos(n)[1] = PS2INCH(ND_coord(n).y);
	}
	spline_edges0(g, true);
    }
    else {
	int et = EDGE_TYPE (g);
	if (et != EDGETYPE_NONE) spline_edges1(g, et);
    }
    dotneato_postprocess(g);
}

static void cleanup_graphs (Agraph_t *g)
{
    for (int i = 1; i <= GD_n_cluster(g); i++) {
        graph_t *const subg = GD_clust(g)[i];
	free_label(GD_label(subg));
        cleanup_graphs (subg);
    }
    free (GD_clust(g));
}

void osage_cleanup(Agraph_t *g)
{
    for (node_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        for (edge_t *e = agfstout(g, n); e; e = agnxtout(g, e)) {
            gv_cleanup_edge(e);
        }
        gv_cleanup_node(n);
    }
    cleanup_graphs(g);
}

/**
 * @dir lib/osage
 * @brief clustered layout engine, API osage/osage.h
 * @ingroup engines
 *
 * [Osage layout user manual](https://graphviz.org/docs/layouts/osage/)
 *
 * Other @ref engines
 */
