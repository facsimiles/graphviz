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

/* comp.c:
 * Written by Emden R. Gansner
 *
 * Support for "connected components". Components are either connected
 * or have a port node or have a pinned node.
 *
 */

/* use PRIVATE interface */
#define FDP_PRIVATE 1

#include <cgraph/cgraph.h>
#include <fdpgen/fdp.h>
#include <fdpgen/comp.h>
#include <pack/pack.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/bitarray.h>
#include <util/list.h>
#include <util/prisize_t.h>

static void dfs(Agraph_t *g, Agnode_t *n, Agraph_t *out, bitarray_t *marks) {
    Agedge_t *e;
    Agnode_t *other;

    bitarray_set(marks, ND_id(n), true);
    agsubnode(out,n,1);
    for (e = agfstedge(g, n); e; e = agnxtedge(g, e, n)) {
	if ((other = agtail(e)) == n)
	    other = aghead(e);
	if (!bitarray_get(*marks, ND_id(other)))
	    dfs(g, other, out, marks);
    }
}

/* findCComp:
 * Finds generalized connected components of graph g.
 * This merges all components containing a port node or a pinned node.
 * Assumes nodes have unique id's in range [0,agnnodes(g)-1].
 * Components are stored as subgraphs of g, with name sg_<i>.
 * Returns 0-terminated array of components.
 * If cnt is non-0, count of components is stored there.
 * If pinned is non-0, *pinned is set to 1 if there are pinned nodes.
 * Note that if ports and/or pinned nodes exists, they will all be
 * in the first component returned by findCComp.
 */
graphs_t findCComp(graph_t *g, int *pinned, size_t *counter) {
    node_t *n;
    graph_t *subg;
    agxbuf name = {0};
    size_t c_cnt = 0;
    bport_t *pp;
    int pinflag = 0;

    assert(agnnodes(g) >= 0);
    bitarray_t marks = bitarray_new((size_t)agnnodes(g));

    /* Create component based on port nodes */
    subg = 0;
    if ((pp = PORTS(g))) {
	agxbprint(&name, "cc%s_%" PRISIZE_T, agnameof(g), c_cnt++ + *counter);
	subg = agsubg(g, agxbuse(&name), 1);
	agbindrec(subg, "Agraphinfo_t", sizeof(Agraphinfo_t), true);
	GD_alg(subg) = gv_alloc(sizeof(gdata));
	PORTS(subg) = pp;
	NPORTS(subg) = NPORTS(g);
	for (; pp->n; pp++) {
	    if (bitarray_get(marks, ND_id(pp->n)))
		continue;
	    dfs(g, pp->n, subg, &marks);
	}
    }

    /* Create/extend component based on pinned nodes */
    /* Note that ports cannot be pinned */
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	if (bitarray_get(marks, ND_id(n)))
	    continue;
	if (ND_pinned(n) != P_PIN)
	    continue;
	if (!subg) {
	    agxbprint(&name, "cc%s_%" PRISIZE_T, agnameof(g), c_cnt++ + *counter);
	    subg = agsubg(g, agxbuse(&name), 1);
		agbindrec(subg, "Agraphinfo_t", sizeof(Agraphinfo_t), true);
	    GD_alg(subg) = gv_alloc(sizeof(gdata));
	}
	pinflag = 1;
	dfs(g, n, subg, &marks);
    }
    if (subg)
	(void)graphviz_node_induce(subg, NULL);

    /* Pick up remaining components */
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	if (bitarray_get(marks, ND_id(n)))
	    continue;
	agxbprint(&name, "cc%s+%" PRISIZE_T, agnameof(g), c_cnt++ + *counter);
	subg = agsubg(g, agxbuse(&name), 1);
	agbindrec(subg, "Agraphinfo_t", sizeof(Agraphinfo_t), true);	//node custom data
	GD_alg(subg) = gv_alloc(sizeof(gdata));
	dfs(g, n, subg, &marks);
	(void)graphviz_node_induce(subg, NULL);
    }
    bitarray_reset(&marks);
    agxbfree(&name);
    *counter += c_cnt;

    if (pinned)
	*pinned = pinflag;
    /* freed in layout */
    graphs_t comps = {0};
    LIST_RESERVE(&comps, c_cnt + 1);
    for (subg = agfstsubg(g); subg; subg = agnxtsubg(subg)) {
	LIST_APPEND(&comps, subg);
	c_cnt--;
    }
    assert(c_cnt == 0);
    LIST_APPEND(&comps, NULL);

    return comps;
}
