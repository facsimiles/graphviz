/**
 * @file
 * @brief Network Simplex algorithm for ranking nodes of a DAG, @ref rank, @ref rank2
 * @ingroup common_render
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <assert.h>
#include <common/render.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/list.h>
#include <util/overflow.h>
#include <util/prisize_t.h>
#include <util/streq.h>

static void dfs_cutval(node_t * v, edge_t * par);
static int dfs_range_init(node_t *v);
static int dfs_range(node_t * v, edge_t * par, int low);
static int x_val(edge_t * e, node_t * v, int dir);
#ifdef DEBUG
static void check_cycles(graph_t * g);
#endif

#define LENGTH(e)		(ND_rank(aghead(e)) - ND_rank(agtail(e)))
#define SLACK(e)		(LENGTH(e) - ED_minlen(e))
#define SEQ(a,b,c)		((a) <= (b) && (b) <= (c))
#define TREE_EDGE(e)	(ED_tree_index(e) >= 0)

DEFINE_LIST(edge_list, edge_t *)

typedef struct {
    graph_t *G;
    nlist_t Tree_node;
    edge_list_t Tree_edge;
    size_t S_i;			/* search index for enter_edge */
    size_t N_edges, N_nodes;
    int Search_size;

    edge_t *Enter;
    int Low, Lim, Slack;
} network_simplex_ctx_t;

#define SEARCHSIZE 30

static int add_tree_edge(network_simplex_ctx_t *ctx, edge_t * e)
{
    node_t *n;
    if (TREE_EDGE(e)) {
	agerrorf("add_tree_edge: missing tree edge\n");
	return -1;
    }
    assert(edge_list_size(&ctx->Tree_edge) <= INT_MAX);
    ED_tree_index(e) = (int)edge_list_size(&ctx->Tree_edge);
    edge_list_append(&ctx->Tree_edge, e);
    if (!ND_mark(agtail(e)))
	ctx->Tree_node.list[ctx->Tree_node.size++] = agtail(e);
    if (!ND_mark(aghead(e)))
	ctx->Tree_node.list[ctx->Tree_node.size++] = aghead(e);
    n = agtail(e);
    ND_mark(n) = true;
    ND_tree_out(n).list[ND_tree_out(n).size++] = e;
    ND_tree_out(n).list[ND_tree_out(n).size] = NULL;
    if (ND_out(n).list[ND_tree_out(n).size - 1] == 0) {
	agerrorf("add_tree_edge: empty outedge list\n");
	return -1;
    }
    n = aghead(e);
    ND_mark(n) = true;
    ND_tree_in(n).list[ND_tree_in(n).size++] = e;
    ND_tree_in(n).list[ND_tree_in(n).size] = NULL;
    if (ND_in(n).list[ND_tree_in(n).size - 1] == 0) {
	agerrorf("add_tree_edge: empty inedge list\n");
	return -1;
    }
    return 0;
}

/**
 * Invalidate DFS attributes by walking up the tree from to_node till lca
 * (inclusively). Called when updating tree to improve pruning in dfs_range().
 * Assigns ND_low(n) = -1 for the affected nodes.
 */
static void invalidate_path(node_t *lca, node_t *to_node) {
    while (true) {
        if (ND_low(to_node) == -1)
          break;

        ND_low(to_node) = -1;

        edge_t *e = ND_par(to_node);
        if (e == NULL)
          break;

        if (ND_lim(to_node) >= ND_lim(lca)) {
          if (to_node != lca)
            agerrorf("invalidate_path: skipped over LCA\n");
          break;
        }

        if (ND_lim(agtail(e)) > ND_lim(aghead(e)))
          to_node = agtail(e);
        else
          to_node = aghead(e);
    }
}

static void exchange_tree_edges(network_simplex_ctx_t *ctx, edge_t * e, edge_t * f)
{
    node_t *n;

    ED_tree_index(f) = ED_tree_index(e);
    assert(ED_tree_index(e) >= 0);
    edge_list_set(&ctx->Tree_edge, (size_t)ED_tree_index(e), f);
    ED_tree_index(e) = -1;

    n = agtail(e);
    size_t i = --ND_tree_out(n).size;
    size_t j;
    for (j = 0; j <= i; j++)
	if (ND_tree_out(n).list[j] == e)
	    break;
    ND_tree_out(n).list[j] = ND_tree_out(n).list[i];
    ND_tree_out(n).list[i] = NULL;
    n = aghead(e);
    i = --ND_tree_in(n).size;
    for (j = 0; j <= i; j++)
	if (ND_tree_in(n).list[j] == e)
	    break;
    ND_tree_in(n).list[j] = ND_tree_in(n).list[i];
    ND_tree_in(n).list[i] = NULL;

    n = agtail(f);
    ND_tree_out(n).list[ND_tree_out(n).size++] = f;
    ND_tree_out(n).list[ND_tree_out(n).size] = NULL;
    n = aghead(f);
    ND_tree_in(n).list[ND_tree_in(n).size++] = f;
    ND_tree_in(n).list[ND_tree_in(n).size] = NULL;
}

DEFINE_LIST(node_queue, node_t *)

static
void init_rank(network_simplex_ctx_t *ctx)
{
    int i;
    node_t *v;
    edge_t *e;

    node_queue_t Q = {0};
    node_queue_reserve(&Q, ctx->N_nodes);
    size_t ctr = 0;

    for (v = GD_nlist(ctx->G); v; v = ND_next(v)) {
	if (ND_priority(v) == 0)
	    node_queue_push_back(&Q, v);
    }

    while (!node_queue_is_empty(&Q)) {
	v = node_queue_pop_front(&Q);
	ND_rank(v) = 0;
	ctr++;
	for (i = 0; (e = ND_in(v).list[i]); i++)
	    ND_rank(v) = MAX(ND_rank(v), ND_rank(agtail(e)) + ED_minlen(e));
	for (i = 0; (e = ND_out(v).list[i]); i++) {
	    if (--ND_priority(aghead(e)) <= 0)
		node_queue_push_back(&Q, aghead(e));
	}
    }
    if (ctr != ctx->N_nodes) {
	agerrorf("trouble in init_rank\n");
	for (v = GD_nlist(ctx->G); v; v = ND_next(v))
	    if (ND_priority(v))
		agerr(AGPREV, "\t%s %d\n", agnameof(v), ND_priority(v));
    }
    node_queue_free(&Q);
}

static edge_t *leave_edge(network_simplex_ctx_t *ctx)
{
    edge_t *f, *rv = NULL;
    int cnt = 0;

    size_t j = ctx->S_i;
    while (ctx->S_i < edge_list_size(&ctx->Tree_edge)) {
	if (ED_cutvalue(f = edge_list_get(&ctx->Tree_edge, ctx->S_i)) < 0) {
	    if (rv) {
		if (ED_cutvalue(rv) > ED_cutvalue(f))
		    rv = f;
	    } else
		rv = edge_list_get(&ctx->Tree_edge, ctx->S_i);
	    if (++cnt >= ctx->Search_size)
		return rv;
	}
	ctx->S_i++;
    }
    if (j > 0) {
	ctx->S_i = 0;
	while (ctx->S_i < j) {
	    if (ED_cutvalue(f = edge_list_get(&ctx->Tree_edge, ctx->S_i)) < 0) {
		if (rv) {
		    if (ED_cutvalue(rv) > ED_cutvalue(f))
			rv = f;
		} else
		    rv = edge_list_get(&ctx->Tree_edge, ctx->S_i);
		if (++cnt >= ctx->Search_size)
		    return rv;
	    }
	    ctx->S_i++;
	}
    }
    return rv;
}

static void dfs_enter_outedge(network_simplex_ctx_t *ctx, node_t * v)
{
    int i, slack;
    edge_t *e;

    for (i = 0; (e = ND_out(v).list[i]); i++) {
	if (!TREE_EDGE(e)) {
	    if (!SEQ(ctx->Low, ND_lim(aghead(e)), ctx->Lim)) {
		slack = SLACK(e);
		if (slack < ctx->Slack || ctx->Enter == NULL) {
		    ctx->Enter = e;
		    ctx->Slack = slack;
		}
	    }
	} else if (ND_lim(aghead(e)) < ND_lim(v))
	    dfs_enter_outedge(ctx, aghead(e));
    }
    for (i = 0; (e = ND_tree_in(v).list[i]) && (ctx->Slack > 0); i++)
	if (ND_lim(agtail(e)) < ND_lim(v))
	    dfs_enter_outedge(ctx, agtail(e));
}

static void dfs_enter_inedge(network_simplex_ctx_t *ctx, node_t * v)
{
    int i, slack;
    edge_t *e;

    for (i = 0; (e = ND_in(v).list[i]); i++) {
	if (!TREE_EDGE(e)) {
	    if (!SEQ(ctx->Low, ND_lim(agtail(e)), ctx->Lim)) {
		slack = SLACK(e);
		if (slack < ctx->Slack || ctx->Enter == NULL) {
		    ctx->Enter = e;
		    ctx->Slack = slack;
		}
	    }
	} else if (ND_lim(agtail(e)) < ND_lim(v))
	    dfs_enter_inedge(ctx, agtail(e));
    }
    for (i = 0; (e = ND_tree_out(v).list[i]) && ctx->Slack > 0; i++)
	if (ND_lim(aghead(e)) < ND_lim(v))
	    dfs_enter_inedge(ctx, aghead(e));
}

static edge_t *enter_edge(network_simplex_ctx_t *ctx, edge_t * e)
{
    node_t *v;
    bool outsearch;

    /* v is the down node */
    if (ND_lim(agtail(e)) < ND_lim(aghead(e))) {
	v = agtail(e);
	outsearch = false;
    } else {
	v = aghead(e);
	outsearch = true;
    }
    ctx->Enter = NULL;
    ctx->Slack = INT_MAX;
    ctx->Low = ND_low(v);
    ctx->Lim = ND_lim(v);
    if (outsearch)
	dfs_enter_outedge(ctx, v);
    else
	dfs_enter_inedge(ctx, v);
    return ctx->Enter;
}

static void init_cutvalues(network_simplex_ctx_t *ctx)
{
    dfs_range_init(GD_nlist(ctx->G));
    dfs_cutval(GD_nlist(ctx->G), NULL);
}

/* functions for initial tight tree construction */
// borrow field from network simplex - overwritten in init_cutvalues() forgive me
#define ND_subtree(n) (subtree_t*)ND_par(n)
#define ND_subtree_set(n,value) (ND_par(n) = (edge_t*)value)

typedef struct subtree_s {
        node_t *rep;            /* some node in the tree */
        int    size;            /* total tight tree size */
        size_t    heap_index; ///< required to find non-min elts when merged
        struct subtree_s *par;  /* union find */
} subtree_t;

/// is this subtree stored in an STheap?
static bool on_heap(const subtree_t *tree) {
  return tree->heap_index != SIZE_MAX;
}

/// state for use in `tight_subtree_search`
typedef struct {
  Agnode_t *v;
  int in_i;  ///< iteration counter through `ND_in(v).list`
  int out_i; ///< iteration counter through `ND_out(v).list`
  int rv;    ///< result value
} tst_t;

/// a stack of states
DEFINE_LIST(tsts, tst_t)

/* find initial tight subtrees */
static int tight_subtree_search(network_simplex_ctx_t *ctx, Agnode_t *v, subtree_t *st)
{
    Agedge_t *e;
    int     rv;

    rv = 1;
    ND_subtree_set(v,st);

    tsts_t todo = {0};
    tsts_push_back(&todo, (tst_t){.v = v, .rv = 1});

    while (!tsts_is_empty(&todo)) {
        bool updated = false;
        tst_t *top = tsts_back(&todo);

        for (; (e = ND_in(top->v).list[top->in_i]); top->in_i++) {
            if (TREE_EDGE(e)) continue;
            if (ND_subtree(agtail(e)) == 0 && SLACK(e) == 0) {
                if (add_tree_edge(ctx, e) != 0) {
                    (void)tsts_pop_back(&todo);
                    if (tsts_is_empty(&todo)) {
                        rv = -1;
                    } else {
                        --tsts_back(&todo)->rv;
                    }
                } else {
                    ND_subtree_set(agtail(e), st);
                    const tst_t next = {.v = agtail(e), .rv = 1};
                    tsts_push_back(&todo, next);
                }
                updated = true;
                break;
            }
        }
        if (updated) {
            continue;
        }

        for (; (e = ND_out(top->v).list[top->out_i]); top->out_i++) {
            if (TREE_EDGE(e)) continue;
            if (ND_subtree(aghead(e)) == 0 && SLACK(e) == 0) {
                if (add_tree_edge(ctx, e) != 0) {
                    (void)tsts_pop_back(&todo);
                    if (tsts_is_empty(&todo)) {
                        rv = -1;
                    } else {
                        --tsts_back(&todo)->rv;
                    }
                } else {
                    ND_subtree_set(aghead(e), st);
                    const tst_t next = {.v = aghead(e), .rv = 1};
                    tsts_push_back(&todo, next);
                }
                updated = true;
                break;
            }
        }
        if (updated) {
          continue;
        }

        const tst_t last = tsts_pop_back(&todo);
        if (tsts_is_empty(&todo)) {
            rv = last.rv;
        } else {
            tsts_back(&todo)->rv += last.rv;
        }
    }

    tsts_free(&todo);

    return rv;
}

static subtree_t *find_tight_subtree(network_simplex_ctx_t *ctx, Agnode_t *v)
{
    subtree_t       *rv;
    rv = gv_alloc(sizeof(subtree_t));
    rv->rep = v;
    rv->size = tight_subtree_search(ctx,v,rv);
    if (rv->size < 0) {
        free(rv);
        return NULL;
    }
    rv->par = rv;
    return rv;
}

typedef struct STheap_s {
        subtree_t       **elt;
        size_t          size;
} STheap_t;

static subtree_t *STsetFind(Agnode_t *n0)
{
  subtree_t *s0 = ND_subtree(n0);
  while  (s0->par && s0->par != s0) {
    if (s0->par->par) {s0->par = s0->par->par;}  /* path compression for the code weary */
    s0 = s0->par;
  }
  return s0;
}
 
static subtree_t *STsetUnion(subtree_t *s0, subtree_t *s1)
{
  subtree_t *r0, *r1, *r;

  for (r0 = s0; r0->par && r0->par != r0; r0 = r0->par);
  for (r1 = s1; r1->par && r1->par != r1; r1 = r1->par);
  if (r0 == r1) return r0;  /* safety code but shouldn't happen */
  assert(on_heap(r0) || on_heap(r1));
  if (!on_heap(r1)) r = r0;
  else if (!on_heap(r0)) r = r1;
  else if (r1->size < r0->size) r = r0;
  else r = r1;

  r0->par = r1->par = r;
  r->size = r0->size + r1->size;
  assert(on_heap(r));
  return r;
}

/* find tightest edge to another tree incident on the given tree */
static Agedge_t *inter_tree_edge_search(Agnode_t *v, Agnode_t *from, Agedge_t *best)
{
    int i;
    Agedge_t *e;
    subtree_t *ts = STsetFind(v);
    if (best && SLACK(best) == 0) return best;
    for (i = 0; (e = ND_out(v).list[i]); i++) {
      if (TREE_EDGE(e)) {
          if (aghead(e) == from) continue;  // do not search back in tree
          best = inter_tree_edge_search(aghead(e),v,best); // search forward in tree
      }
      else {
        if (STsetFind(aghead(e)) != ts) {   // encountered candidate edge
          if (best == 0 || SLACK(e) < SLACK(best)) best = e;
        }
        /* else ignore non-tree edge between nodes in the same tree */
      }
    }
    /* the following code must mirror the above, but for in-edges */
    for (i = 0; (e = ND_in(v).list[i]); i++) {
      if (TREE_EDGE(e)) {
          if (agtail(e) == from) continue;
          best = inter_tree_edge_search(agtail(e),v,best);
      }
      else {
        if (STsetFind(agtail(e)) != ts) {
          if (best == 0 || SLACK(e) < SLACK(best)) best = e;
        }
      }
    }
    return best;
}

static Agedge_t *inter_tree_edge(subtree_t *tree)
{
    Agedge_t *rv;
    rv = inter_tree_edge_search(tree->rep, NULL, NULL);
    return rv;
}

static size_t STheapsize(const STheap_t *heap) { return heap->size; }

static void STheapify(STheap_t *heap, size_t i) {
    subtree_t **elt = heap->elt;
    do {
        const size_t left = 2 * (i + 1) - 1;
        const size_t right = 2 * (i + 1);
        size_t smallest = i;
        if (left < heap->size && elt[left]->size < elt[smallest]->size) smallest = left;
        if (right < heap->size && elt[right]->size < elt[smallest]->size) smallest = right;
        if (smallest != i) {
            subtree_t *temp;
            temp = elt[i];
            elt[i] = elt[smallest];
            elt[smallest] = temp;
            elt[i]->heap_index = i;
            elt[smallest]->heap_index = smallest;
            i = smallest;
        }
        else break;
    } while (i < heap->size);
}

static STheap_t *STbuildheap(subtree_t **elt, size_t size) {
    STheap_t *heap = gv_alloc(sizeof(STheap_t));
    heap->elt = elt;
    heap->size = size;
    for (size_t i = 0; i < heap->size; i++) heap->elt[i]->heap_index = i;
    for (size_t i = heap->size / 2; i != SIZE_MAX; i--)
        STheapify(heap,i);
    return heap;
}

static
subtree_t *STextractmin(STheap_t *heap)
{
    subtree_t *rv;
    rv = heap->elt[0];
    rv->heap_index = SIZE_MAX;
      // mark this as not participating in the heap anymore
    heap->elt[0] = heap->elt[heap->size - 1];
    heap->elt[0]->heap_index = 0;
    heap->elt[heap->size -1] = rv;    /* needed to free storage later */
    heap->size--;
    STheapify(heap,0);
    return rv;
}

static
void tree_adjust(Agnode_t *v, Agnode_t *from, int delta)
{
    int i;
    Agedge_t *e;
    Agnode_t *w;
    ND_rank(v) = ND_rank(v) + delta;
    for (i = 0; (e = ND_tree_in(v).list[i]); i++) {
      w = agtail(e);
      if (w != from)
        tree_adjust(w, v, delta);
    }
    for (i = 0; (e = ND_tree_out(v).list[i]); i++) {
      w = aghead(e);
      if (w != from)
        tree_adjust(w, v, delta);
    }
}

static
subtree_t *merge_trees(network_simplex_ctx_t *ctx, Agedge_t *e)   /* entering tree edge */
{
  int       delta;
  subtree_t *t0, *t1, *rv;

  assert(!TREE_EDGE(e));

  t0 = STsetFind(agtail(e));
  t1 = STsetFind(aghead(e));

  if (!on_heap(t0)) { // move t0
    delta = SLACK(e);
    if (delta != 0)
      tree_adjust(t0->rep,NULL,delta);
  }
  else {  // move t1
    delta = -SLACK(e);
    if (delta != 0)
      tree_adjust(t1->rep,NULL,delta);
  }
  if (add_tree_edge(ctx, e) != 0) {
    return NULL;
  }
  rv = STsetUnion(t0,t1);
  
  return rv;
}

/* Construct initial tight tree. Graph must be connected, feasible.
 * Adjust ND_rank(v) as needed.  add_tree_edge() on tight tree edges.
 * trees are basically lists of nodes stored in `node_queue_t`s.
 * Return 1 if input graph is not connected; 0 on success.
 */
static
int feasible_tree(network_simplex_ctx_t *ctx)
{
  Agedge_t *ee;
  size_t subtree_count = 0;
  STheap_t *heap = NULL;
  int error = 0;

  /* initialization */
  for (Agnode_t *n = GD_nlist(ctx->G); n != NULL; n = ND_next(n)) {
      ND_subtree_set(n,0);
  }

  subtree_t **tree = gv_calloc(ctx->N_nodes, sizeof(subtree_t *));
  /* given init_rank, find all tight subtrees */
  for (Agnode_t *n = GD_nlist(ctx->G); n != NULL; n = ND_next(n)) {
        if (ND_subtree(n) == 0) {
                tree[subtree_count] = find_tight_subtree(ctx, n);
                if (tree[subtree_count] == NULL) {
                    error = 2;
                    goto end;
                }
                subtree_count++;
        }
  }

  /* incrementally merge subtrees */
  heap = STbuildheap(tree,subtree_count);
  while (STheapsize(heap) > 1) {
    subtree_t *tree0 = STextractmin(heap);
    if (!(ee = inter_tree_edge(tree0))) {
      error = 1;
      break;
    }
    subtree_t *tree1 = merge_trees(ctx, ee);
    if (tree1 == NULL) {
      error = 2;
      break;
    }
    STheapify(heap,tree1->heap_index);
  }

end:
  free(heap);
  for (size_t i = 0; i < subtree_count; i++) free(tree[i]);
  free(tree);
  if (error) return error;
  assert(edge_list_size(&ctx->Tree_edge) == ctx->N_nodes - 1);
  init_cutvalues(ctx);
  return 0;
}

/* walk up from v to LCA(v,w), setting new cutvalues. */
static Agnode_t *treeupdate(Agnode_t * v, Agnode_t * w, int cutvalue, int dir)
{
    edge_t *e;
    int d;

    while (!SEQ(ND_low(v), ND_lim(w), ND_lim(v))) {
	e = ND_par(v);
	if (v == agtail(e))
	    d = dir;
	else
	    d = !dir;
	if (d)
	    ED_cutvalue(e) += cutvalue;
	else
	    ED_cutvalue(e) -= cutvalue;
	if (ND_lim(agtail(e)) > ND_lim(aghead(e)))
	    v = agtail(e);
	else
	    v = aghead(e);
    }
    return v;
}

static void rerank(Agnode_t * v, int delta)
{
    int i;
    edge_t *e;

    ND_rank(v) -= delta;
    for (i = 0; (e = ND_tree_out(v).list[i]); i++)
	if (e != ND_par(v))
	    rerank(aghead(e), delta);
    for (i = 0; (e = ND_tree_in(v).list[i]); i++)
	if (e != ND_par(v))
	    rerank(agtail(e), delta);
}

/* e is the tree edge that is leaving and f is the nontree edge that
 * is entering.  compute new cut values, ranks, and exchange e and f.
 */
static int
update(network_simplex_ctx_t *ctx, edge_t * e, edge_t * f)
{
    int cutvalue, delta;
    Agnode_t *lca;

    delta = SLACK(f);
    /* "for (v = in nodes in tail side of e) do ND_rank(v) -= delta;" */
    if (delta > 0) {
	size_t s = ND_tree_in(agtail(e)).size + ND_tree_out(agtail(e)).size;
	if (s == 1)
	    rerank(agtail(e), delta);
	else {
	    s = ND_tree_in(aghead(e)).size + ND_tree_out(aghead(e)).size;
	    if (s == 1)
		rerank(aghead(e), -delta);
	    else {
		if (ND_lim(agtail(e)) < ND_lim(aghead(e)))
		    rerank(agtail(e), delta);
		else
		    rerank(aghead(e), -delta);
	    }
	}
    }

    cutvalue = ED_cutvalue(e);
    lca = treeupdate(agtail(f), aghead(f), cutvalue, 1);
    if (treeupdate(aghead(f), agtail(f), cutvalue, 0) != lca) {
	agerrorf("update: mismatched lca in treeupdates\n");
	return 2;
    }

    // invalidate paths from LCA till affected nodes:
    int lca_low = ND_low(lca);
    invalidate_path(lca, aghead(f));
    invalidate_path(lca, agtail(f));

    ED_cutvalue(f) = -cutvalue;
    ED_cutvalue(e) = 0;
    exchange_tree_edges(ctx, e, f);
    dfs_range(lca, ND_par(lca), lca_low);
    return 0;
}

static int scan_and_normalize(network_simplex_ctx_t *ctx) {
    node_t *n;

    int Minrank = INT_MAX;
    int Maxrank = INT_MIN;
    for (n = GD_nlist(ctx->G); n; n = ND_next(n)) {
	if (ND_node_type(n) == NORMAL) {
	    Minrank = MIN(Minrank, ND_rank(n));
	    Maxrank = MAX(Maxrank, ND_rank(n));
	}
    }
    for (n = GD_nlist(ctx->G); n; n = ND_next(n))
	ND_rank(n) -= Minrank;
    Maxrank -= Minrank;
    return Maxrank;
}

static void reset_lists(network_simplex_ctx_t *ctx) {

  free(ctx->Tree_node.list);
  ctx->Tree_node = (nlist_t){0};

  edge_list_free(&ctx->Tree_edge);
}

static void
freeTreeList (network_simplex_ctx_t *ctx, graph_t* g)
{
    node_t *n;
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	free_list(ND_tree_in(n));
	free_list(ND_tree_out(n));
	ND_mark(n) = false;
    }
    reset_lists(ctx);
}

static void LR_balance(network_simplex_ctx_t *ctx)
{
    int delta;
    edge_t *e, *f;

    for (size_t i = 0; i < edge_list_size(&ctx->Tree_edge); i++) {
	e = edge_list_get(&ctx->Tree_edge, i);
	if (ED_cutvalue(e) == 0) {
	    f = enter_edge(ctx,e);
	    if (f == NULL)
		continue;
	    delta = SLACK(f);
	    if (delta <= 1)
		continue;
	    if (ND_lim(agtail(e)) < ND_lim(aghead(e)))
		rerank(agtail(e), delta / 2);
	    else
		rerank(aghead(e), -delta / 2);
	}
    }
    freeTreeList(ctx, ctx->G);
}

static int decreasingrankcmpf(const void *x, const void *y) {
// Suppress Clang/GCC -Wcast-qual warning. Casting away const here is acceptable
// as the later usage is const. We need the cast because the macros use
// non-const pointers for genericity.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
  node_t **n0 = (node_t**)x;
  node_t **n1 = (node_t**)y;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
  if (ND_rank(*n1) < ND_rank(*n0)) {
    return -1;
  }
  if (ND_rank(*n1) > ND_rank(*n0)) {
    return 1;
  }
  return 0;
}

static int increasingrankcmpf(const void *x, const void *y) {
// Suppress Clang/GCC -Wcast-qual warning. Casting away const here is acceptable
// as the later usage is const. We need the cast because the macros use
// non-const pointers for genericity.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
  node_t **n0 = (node_t **)x;
  node_t **n1 = (node_t **)y;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
  if (ND_rank(*n0) < ND_rank(*n1)) {
    return -1;
  }
  if (ND_rank(*n0) > ND_rank(*n1)) {
    return 1;
  }
  return 0;
}

static void TB_balance(network_simplex_ctx_t *ctx)
{
    node_t *n;
    edge_t *e;
    int low, high, choice;
    int inweight, outweight;
    int adj = 0;
    char *s;

    const int Maxrank = scan_and_normalize(ctx);

    /* find nodes that are not tight and move to less populated ranks */
    assert(Maxrank >= 0);
    int *nrank = gv_calloc((size_t)Maxrank + 1, sizeof(int));
    if ( (s = agget(ctx->G,"TBbalance")) ) {
         if (streq(s,"min")) adj = 1;
         else if (streq(s,"max")) adj = 2;
         if (adj) for (n = GD_nlist(ctx->G); n; n = ND_next(n))
              if (ND_node_type(n) == NORMAL) {
                if (ND_in(n).size == 0 && adj == 1) {
                   ND_rank(n) = 0;
                }
                if (ND_out(n).size == 0 && adj == 2) {
                   ND_rank(n) = Maxrank;
                }
              }
    }
    size_t ii;
    for (ii = 0, n = GD_nlist(ctx->G); n; ii++, n = ND_next(n)) {
      ctx->Tree_node.list[ii] = n;
    }
    ctx->Tree_node.size = ii;
    qsort(ctx->Tree_node.list, ctx->Tree_node.size, sizeof(ctx->Tree_node.list[0]),
          adj > 1 ? decreasingrankcmpf: increasingrankcmpf);
    for (size_t i = 0; i < ctx->Tree_node.size; i++) {
        n = ctx->Tree_node.list[i];
        if (ND_node_type(n) == NORMAL)
          nrank[ND_rank(n)]++;
    }
    for (ii = 0; ii < ctx->Tree_node.size; ii++) {
      n = ctx->Tree_node.list[ii];
      if (ND_node_type(n) != NORMAL)
        continue;
      inweight = outweight = 0;
      low = 0;
      high = Maxrank;
      for (size_t i = 0; (e = ND_in(n).list[i]); i++) {
        inweight += ED_weight(e);
        low = MAX(low, ND_rank(agtail(e)) + ED_minlen(e));
      }
      for (size_t i = 0; (e = ND_out(n).list[i]); i++) {
        outweight += ED_weight(e);
        high = MIN(high, ND_rank(aghead(e)) - ED_minlen(e));
      }
      if (low < 0)
        low = 0;		/* vnodes can have ranks < 0 */
      if (adj) {
        if (inweight == outweight)
            ND_rank(n) = (adj == 1? low : high);
      }
      else {
                if (inweight == outweight) {
                    choice = low;
                    for (int i = low + 1; i <= high; i++)
                        if (nrank[i] < nrank[choice])
                            choice = i;
                    nrank[ND_rank(n)]--;
                    nrank[choice]++;
                    ND_rank(n) = choice;
                }
      }
      free_list(ND_tree_in(n));
      free_list(ND_tree_out(n));
      ND_mark(n) = false;
    }
    free(nrank);
}

static bool init_graph(network_simplex_ctx_t *ctx, graph_t *g) {
    node_t *n;
    edge_t *e;

    ctx->G = g;
    ctx->N_nodes = ctx->N_edges = ctx->S_i = 0;
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	ND_mark(n) = false;
	ctx->N_nodes++;
	for (size_t i = 0; (e = ND_out(n).list[i]); i++)
	    ctx->N_edges++;
    }

    ctx->Tree_node.list = gv_calloc(ctx->N_nodes, sizeof(node_t *));
    edge_list_reserve(&ctx->Tree_edge, ctx->N_nodes);

    bool feasible = true;
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	ND_priority(n) = 0;
	size_t i;
	for (i = 0; (e = ND_in(n).list[i]); i++) {
	    ND_priority(n)++;
	    ED_cutvalue(e) = 0;
	    ED_tree_index(e) = -1;
	    if (ND_rank(aghead(e)) - ND_rank(agtail(e)) < ED_minlen(e))
		feasible = false;
	}
	ND_tree_in(n).list = gv_calloc(i + 1, sizeof(edge_t *));
	ND_tree_in(n).size = 0;
	for (i = 0; (e = ND_out(n).list[i]); i++);
	ND_tree_out(n).list = gv_calloc(i + 1, sizeof(edge_t *));
	ND_tree_out(n).size = 0;
    }
    return feasible;
}

/* graphSize:
 * Compute no. of nodes and edges in the graph
 */
static void
graphSize (graph_t * g, int* nn, int* ne)
{
    int i, nnodes, nedges;
    node_t *n;
    edge_t *e;
   
    nnodes = nedges = 0;
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	nnodes++;
	for (i = 0; (e = ND_out(n).list[i]); i++) {
	    nedges++;
	}
    }
    *nn = nnodes;
    *ne = nedges;
}

/* rank:
 * Apply network simplex to rank the nodes in a graph.
 * Uses ED_minlen as the internode constraint: if a->b with minlen=ml,
 * rank b - rank a >= ml.
 * Assumes the graph has the following additional structure:
 *   A list of all nodes, starting at GD_nlist, and linked using ND_next.
 *   Out and in edges lists stored in ND_out and ND_in, even if the node
 *  doesn't have any out or in edges.
 * The node rank values are stored in ND_rank.
 * Returns 0 if successful; returns 1 if the graph was not connected;
 * returns 2 if something seriously wrong;
 */
int rank2(graph_t * g, int balance, int maxiter, int search_size)
{
    int iter = 0;
    char *ns = "network simplex: ";
    edge_t *e, *f;
    network_simplex_ctx_t ctx = {0};

#ifdef DEBUG
    check_cycles(g);
#endif
    if (Verbose) {
	int nn, ne;
	graphSize (g, &nn, &ne);
	fprintf(stderr, "%s %d nodes %d edges maxiter=%d balance=%d\n", ns,
	    nn, ne, maxiter, balance);
	start_timer();
    }
    bool feasible = init_graph(&ctx, g);
    if (!feasible)
	init_rank(&ctx);

    if (search_size >= 0)
	ctx.Search_size = search_size;
    else
	ctx.Search_size = SEARCHSIZE;

    {
	int err = feasible_tree(&ctx);
	if (err != 0) {
	    freeTreeList(&ctx, g);
	    return err;
	}
    }
    if (maxiter <= 0) {
	freeTreeList(&ctx, g);
	return 0;
    }

    while ((e = leave_edge(&ctx))) {
	int err;
	f = enter_edge(&ctx, e);
	err = update(&ctx, e, f);
	if (err != 0) {
	    freeTreeList(&ctx, g);
	    return err;
	}
	iter++;
	if (Verbose && iter % 100 == 0) {
	    if (iter % 1000 == 100)
		fputs(ns, stderr);
	    fprintf(stderr, "%d ", iter);
	    if (iter % 1000 == 0)
		fputc('\n', stderr);
	}
	if (iter >= maxiter)
	    break;
    }
    switch (balance) {
    case 1:
	TB_balance(&ctx);
	reset_lists(&ctx);
	break;
    case 2:
	LR_balance(&ctx);
	break;
    default:
	(void)scan_and_normalize(&ctx);
	freeTreeList (&ctx, ctx.G);
	break;
    }
    if (Verbose) {
	if (iter >= 100)
	    fputc('\n', stderr);
	fprintf(stderr, "%s%" PRISIZE_T " nodes %" PRISIZE_T " edges %d iter %.2f sec\n",
		ns, ctx.N_nodes, ctx.N_edges, iter, elapsed_sec());
    }
    return 0;
}

int rank(graph_t * g, int balance, int maxiter)
{
    char *s;
    int search_size;

    if ((s = agget(g, "searchsize")))
	search_size = atoi(s);
    else
	search_size = SEARCHSIZE;

    return rank2 (g, balance, maxiter, search_size);
}

/* set cut value of f, assuming values of edges on one side were already set */
static void x_cutval(edge_t * f)
{
    node_t *v;
    edge_t *e;
    int i, sum, dir;

    /* set v to the node on the side of the edge already searched */
    if (ND_par(agtail(f)) == f) {
	v = agtail(f);
	dir = 1;
    } else {
	v = aghead(f);
	dir = -1;
    }

    sum = 0;
    for (i = 0; (e = ND_out(v).list[i]); i++)
	if (sadd_overflow(sum, x_val(e, v, dir), &sum)) {
	    agerrorf("overflow when computing edge weight sum\n");
	    graphviz_exit(EXIT_FAILURE);
	}
    for (i = 0; (e = ND_in(v).list[i]); i++)
	if (sadd_overflow(sum, x_val(e, v, dir), &sum)) {
	    agerrorf("overflow when computing edge weight sum\n");
	    graphviz_exit(EXIT_FAILURE);
	}
    ED_cutvalue(f) = sum;
}

static int x_val(edge_t * e, node_t * v, int dir)
{
    node_t *other;
    int d, rv, f;

    if (agtail(e) == v)
	other = aghead(e);
    else
	other = agtail(e);
    if (!(SEQ(ND_low(v), ND_lim(other), ND_lim(v)))) {
	f = 1;
	rv = ED_weight(e);
    } else {
	f = 0;
	if (TREE_EDGE(e))
	    rv = ED_cutvalue(e);
	else
	    rv = 0;
	rv -= ED_weight(e);
    }
    if (dir > 0) {
	if (aghead(e) == v)
	    d = 1;
	else
	    d = -1;
    } else {
	if (agtail(e) == v)
	    d = 1;
	else
	    d = -1;
    }
    if (f)
	d = -d;
    if (d < 0)
	rv = -rv;
    return rv;
}

static void dfs_cutval(node_t * v, edge_t * par)
{
    int i;
    edge_t *e;

    for (i = 0; (e = ND_tree_out(v).list[i]); i++)
	if (e != par)
	    dfs_cutval(aghead(e), e);
    for (i = 0; (e = ND_tree_in(v).list[i]); i++)
	if (e != par)
	    dfs_cutval(agtail(e), e);
    if (par)
	x_cutval(par);
}

/// local state used by `dfs_range*`
typedef struct {
  node_t *v;
  edge_t *par;
  int lim;
  int tree_out_i;
  int tree_in_i;
} dfs_state_t;

DEFINE_LIST(dfs_stack, dfs_state_t)

/*
* Initializes DFS range attributes (par, low, lim) over tree nodes such that:
* ND_par(n) - parent tree edge
* ND_low(n) - min DFS index for nodes in sub-tree (>= 1)
* ND_lim(n) - max DFS index for nodes in sub-tree
*/
static int dfs_range_init(node_t *v) {
    int lim = 0;

    dfs_stack_t todo = {0};

    ND_par(v) = NULL;
    ND_low(v) = 1;
    const dfs_state_t root = {.v = v, .par = NULL, .lim = 1};
    dfs_stack_push_back(&todo, root);

    while (!dfs_stack_is_empty(&todo)) {
        bool pushed_new = false;
        dfs_state_t *const s = dfs_stack_back(&todo);

        while (ND_tree_out(s->v).list[s->tree_out_i]) {
            edge_t *const e = ND_tree_out(s->v).list[s->tree_out_i];
            ++s->tree_out_i;
            if (e != s->par) {
                node_t *const n = aghead(e);
                ND_par(n) = e;
                ND_low(n) = s->lim;
                const dfs_state_t next = {.v = n, .par = e, .lim = s->lim};
                dfs_stack_push_back(&todo, next);
                pushed_new = true;
                break;
            }
        }
        if (pushed_new) {
            continue;
        }

        while (ND_tree_in(s->v).list[s->tree_in_i]) {
            edge_t *const e = ND_tree_in(s->v).list[s->tree_in_i];
            ++s->tree_in_i;
            if (e != s->par) {
                node_t *const n = agtail(e);
                ND_par(n) = e;
                ND_low(n) = s->lim;
                const dfs_state_t next = {.v = n, .par = e, .lim = s->lim};
                dfs_stack_push_back(&todo, next);
                pushed_new = true;
                break;
            }
        }
        if (pushed_new) {
            continue;
        }

        ND_lim(s->v) = s->lim;

        lim = s->lim;
        (void)dfs_stack_pop_back(&todo);

        if (!dfs_stack_is_empty(&todo)) {
            dfs_stack_back(&todo)->lim = lim + 1;
        }
    }

    dfs_stack_free(&todo);

    return lim + 1;
}

/*
 * Incrementally updates DFS range attributes
 */
static int dfs_range(node_t * v, edge_t * par, int low)
{
    int lim = 0;

    if (ND_par(v) == par && ND_low(v) == low) {
	return ND_lim(v) + 1;
    }

    dfs_stack_t todo = {0};

    ND_par(v) = par;
    ND_low(v) = low;
    const dfs_state_t root = {.v = v, .par = par, .lim = low};
    dfs_stack_push_back(&todo, root);

    while (!dfs_stack_is_empty(&todo)) {
	bool processed_child = false;
	dfs_state_t *const s = dfs_stack_back(&todo);

	while (ND_tree_out(s->v).list[s->tree_out_i]) {
	    edge_t *const e = ND_tree_out(s->v).list[s->tree_out_i];
	    ++s->tree_out_i;
	    if (e != s->par) {
		node_t *const n = aghead(e);
		if (ND_par(n) == e && ND_low(n) == s->lim) {
		    s->lim = ND_lim(n) + 1;
		} else {
		    ND_par(n) = e;
		    ND_low(n) = s->lim;
		    const dfs_state_t next = {.v = n, .par = e, .lim = s->lim};
		    dfs_stack_push_back(&todo, next);
		}
		processed_child = true;
		break;
	    }
	}
	if (processed_child) {
	    continue;
	}

	while (ND_tree_in(s->v).list[s->tree_in_i]) {
	    edge_t *const e = ND_tree_in(s->v).list[s->tree_in_i];
	    ++s->tree_in_i;
	    if (e != s->par) {
		node_t *const n = agtail(e);
		if (ND_par(n) == e && ND_low(n) == s->lim) {
		    s->lim = ND_lim(n) + 1;
		} else {
		    ND_par(n) = e;
		    ND_low(n) = s->lim;
		    const dfs_state_t next = {.v = n, .par = e, .lim = s->lim};
		    dfs_stack_push_back(&todo, next);
		}
		processed_child = true;
		break;
	    }
	}
	if (processed_child) {
	    continue;
	}

	ND_lim(s->v) = s->lim;

	lim = s->lim;
	(void)dfs_stack_pop_back(&todo);

	if (!dfs_stack_is_empty(&todo)) {
	    dfs_stack_back(&todo)->lim = lim + 1;
	}
    }

    dfs_stack_free(&todo);

    return lim + 1;
}

#ifdef DEBUG
void tchk(network_simplex_ctx_t *ctx)
{
    int i;
    node_t *n;
    edge_t *e;

    size_t n_cnt = 0;
    size_t e_cnt = 0;
    for (n = agfstnode(ctx->G); n; n = agnxtnode(ctx->G, n)) {
	n_cnt++;
	for (i = 0; (e = ND_tree_out(n).list[i]); i++) {
	    e_cnt++;
	    if (SLACK(e) > 0)
		fprintf(stderr, "not a tight tree %p", e);
	}
    }
    if (n_cnt != ctx->Tree_node.size || e_cnt != edge_list_size(&ctx->Tree_edge))
	fprintf(stderr, "something missing\n");
}

void check_fast_node(node_t * n)
{
    node_t *nptr;
    nptr = GD_nlist(agraphof(n));
    while (nptr && nptr != n)
	nptr = ND_next(nptr);
    assert(nptr != NULL);
}

static void dump_node(FILE *sink, node_t *n) {
    if (ND_node_type(n)) {
      fprintf(sink, "%p", n);
    }
    else
      fputs(agnameof(n), sink);
}

static void dump_graph (graph_t* g)
{
    int i;
    edge_t *e;
    node_t *n,*w;
    FILE* fp = fopen ("ns.gv", "w");
    fprintf (fp, "digraph \"%s\" {\n", agnameof(g));
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	fputs("  \"", fp);
	dump_node(fp, n);
	fputs("\"\n", fp);
    }
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	for (i = 0; (e = ND_out(n).list[i]); i++) {
	    fputs("  \"", fp);
	    dump_node(fp, n);
	    fputs("\"", fp);
	    w = aghead(e);
	    fputs(" -> \"", fp);
	    dump_node(fp, w);
	    fputs("\"\n", fp);
	}
    }

    fprintf (fp, "}\n");
    fclose (fp);
}

static node_t *checkdfs(graph_t* g, node_t * n)
{
    edge_t *e;
    node_t *w,*x;
    int i;

    if (ND_mark(n))
	return 0;
    ND_mark(n) = true;
    ND_onstack(n) = true;
    for (i = 0; (e = ND_out(n).list[i]); i++) {
	w = aghead(e);
	if (ND_onstack(w)) {
	    dump_graph (g);
	    fprintf(stderr, "cycle: last edge %lx %s(%lx) %s(%lx)\n",
		(uint64_t)e,
	       	agnameof(n), (uint64_t)n,
		agnameof(w), (uint64_t)w);
	    return w;
	}
	else {
	    if (!ND_mark(w)) {
		x = checkdfs(g, w);
		if (x) {
		    fprintf(stderr,"unwind %lx %s(%lx)\n",
			(uint64_t)e,
			agnameof(n), (uint64_t)n);
		    if (x != n) return x;
		    fprintf(stderr,"unwound to root\n");
		    fflush(stderr);
		    abort();
		    return 0;
		}
	    }
	}
    }
    ND_onstack(n) = false;
    return 0;
}

void check_cycles(graph_t * g)
{
    node_t *n;
    for (n = GD_nlist(g); n; n = ND_next(n)) {
	ND_mark(n) = false;
	ND_onstack(n) = false;
    }
    for (n = GD_nlist(g); n; n = ND_next(n))
	checkdfs(g, n);
}
#endif				/* DEBUG */
