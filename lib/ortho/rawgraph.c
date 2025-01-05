/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <ortho/rawgraph.h>
#include <stdbool.h>
#include <stddef.h>
#include <util/alloc.h>
#include <util/list.h>

#define UNSCANNED 0
#define SCANNING  1
#define SCANNED   2

rawgraph *make_graph(size_t n) {
    rawgraph* g = gv_alloc(sizeof(rawgraph));
    g->nvs = n;
    g->vertices = gv_calloc(n, sizeof(vertex));
    for(size_t i = 0; i < n; ++i) {
        g->vertices[i].color = UNSCANNED;
    }
    return g;
}

void
free_graph(rawgraph* g)
{
    for(size_t i = 0; i < g->nvs; ++i)
        adj_list_free(&g->vertices[i].adj_list);
    free (g->vertices);
    free (g);
}
 
void insert_edge(rawgraph *g, size_t v1, size_t v2) {
    if (!edge_exists(g, v1, v2)) {
      adj_list_append(&g->vertices[v1].adj_list, v2);
    }
}

void remove_redge(rawgraph *g, size_t v1, size_t v2) {
    adj_list_remove(&g->vertices[v1].adj_list, v2);
    adj_list_remove(&g->vertices[v2].adj_list, v1);
}

static bool zeq(size_t a, size_t b) {
  return a == b;
}

bool edge_exists(rawgraph *g, size_t v1, size_t v2) {
  return adj_list_contains(&g->vertices[v1].adj_list, v2, zeq);
}

DEFINE_LIST(int_stack, size_t)

static int DFS_visit(rawgraph *g, size_t v, int time, int_stack_t *sp) {
    vertex* vp;

    vp = g->vertices + v;
    vp->color = SCANNING;
    const adj_list_t adj = vp->adj_list;
    time = time + 1;

    for (size_t i = 0; i < adj_list_size(&adj); ++i) {
        const size_t id = adj_list_get(&adj, i);
        if(g->vertices[id].color == UNSCANNED)
            time = DFS_visit(g, id, time, sp);
    }
    vp->color = SCANNED;
    int_stack_push_back(sp, v);
    return time + 1;
}

void
top_sort(rawgraph* g)
{
    int time = 0;
    int count = 0;

    if (g->nvs == 0) return;
    if (g->nvs == 1) {
        g->vertices[0].topsort_order = count;
		return;
	}

    int_stack_t sp = {0};
    int_stack_reserve(&sp, g->nvs);
    for(size_t i = 0; i < g->nvs; ++i) {
        if(g->vertices[i].color == UNSCANNED)
            time = DFS_visit(g, i, time, &sp);
    }
    while (!int_stack_is_empty(&sp)) {
        const size_t v = int_stack_pop_back(&sp);
        g->vertices[v].topsort_order = count;
        count++;
    }
    int_stack_free(&sp);
}
