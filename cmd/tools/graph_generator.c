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
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <graph_generator.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/list.h>
#include <util/random.h>
#include <util/overflow.h>

void makePath(unsigned n, edgefn ef){
    if (n == 1) {
	ef (1, 0);
	return;
    }
    for (unsigned i = 2; i <= n; i++)
	ef (i - 1, i);
}

void makeComplete(unsigned n, edgefn ef) {
    if (n == 1) {
	ef (1, 0);
	return;
    }
    for (unsigned i = 1; i < n; i++) {
	for (unsigned j = i + 1; j <= n; j++) {
	    ef ( i, j);
	}
    }
}

void makeCircle(unsigned n, edgefn ef) {
    if (n < 3) {
	fprintf(stderr, "Warning: degenerate circle of %u vertices\n", n);
	makePath(n, ef);
	return;
    }

    for (unsigned i = 1; i < n; i++)
	ef ( i, i + 1);
    ef (1, n);
}

void makeStar(unsigned n, edgefn ef) {
    if (n < 3) {
	fprintf(stderr, "Warning: degenerate star of %u vertices\n", n);
	makePath(n, ef);
	return;
    }

    for (unsigned i = 2; i <= n; i++)
	ef (1, i);
}

void makeWheel(unsigned n, edgefn ef) {
    if (n < 4) {
	fprintf(stderr, "Warning: degenerate wheel of %u vertices\n", n);
	makeComplete(n, ef);
	return;
    }

    makeStar(n, ef);

    for (unsigned i = 2; i < n; i++)
	ef( i, i + 1);
    ef (2, n);
}

void makeCompleteB(unsigned dim1, unsigned dim2, edgefn ef) {
    for (unsigned i = 1; i <= dim1; i++) {
	for (unsigned j = 1; j <= dim2; j++) {
	    ef ( i, dim1 + j);
	}
    }
}

void makeTorus(unsigned dim1, unsigned dim2, edgefn ef) {
    for (unsigned i = 1, n = 0; i <= dim1; i++) {
	for (unsigned j = 1; j < dim2; j++) {
	    ef( n + j, n + j + 1);
	}
	ef( n + 1, n + dim2);
	n += dim2;
    }

    for (unsigned i = 1; i <= dim2; i++) {
	for (unsigned j = 1; j < dim1; j++) {
	    ef( dim2 * (j - 1) + i, dim2 * j + i);
	}
	ef( i, dim2 * (dim1 - 1) + i);
    }
}

void makeTwistedTorus(unsigned dim1, unsigned dim2, unsigned t1, unsigned t2,
                      edgefn ef) {
    for (unsigned i = 0; i < dim1; i++) {
	for (unsigned j = 0; j < dim2; j++) {
	    unsigned li = (i + t1) % dim1;
	    unsigned lj = (j + 1) % dim2;
	    ef (i+j*dim1+1, li+lj*dim1+1);

	    li = (i + 1) % dim1;
	    lj = (j + t2) % dim2;
	    ef(i+j*dim1+1, li+lj*dim1+1);
	}
    }
}

void makeCylinder(unsigned dim1, unsigned dim2, edgefn ef) {
    for (unsigned i = 1, n = 0; i <= dim1; i++) {
	for (unsigned j = 1; j < dim2; j++) {
	    ef( n + j, n + j + 1);
	}
	ef( n + 1, n + dim2);
	n += dim2;
    }

    for (unsigned i = 1; i <= dim2; i++) {
	for (unsigned j = 1; j < dim1; j++) {
	    ef( dim2 * (j - 1) + i, dim2 * j + i);
	}
    }
}

#define OUTE(h) if (tl < (hd=(h))) ef( tl, hd)

void makeSquareGrid(unsigned dim1, unsigned dim2, int connect_corners, int partial, edgefn ef)
{
    for (unsigned i = 0; i < dim1; i++)
	for (unsigned j = 0; j < dim2; j++) {
	    // write the neighbors of the node i*dim2+j+1
	    const unsigned tl = i * dim2 + j + 1;
	    unsigned hd;
	    if (j + 1 < dim2
		&& (!partial || j < 2 * dim2 / 6 || j >= 4 * dim2 / 6
		    || i <= 2 * dim1 / 6 || i > 4 * dim1 / 6)) {
		ef(tl, i * dim2 + j + 2);
	    }
	    if (i + 1 < dim1) {
		ef(tl, (i + 1) * dim2 + j + 1);
	    }
	    if (connect_corners == 1) {
		if (i == 0 && j == 0) {	// upper left
		    OUTE((dim1 - 1) * dim2 + dim2);
		} else if (i + 1 == dim1 && j == 0) { // lower left
		    OUTE(dim2);
		} else if (i == 0 && j + 1 == dim2) { // upper right
		    OUTE((dim1 - 1) * dim2 + 1);
		} else if (i + 1 == dim1 && j + 1 == dim2) { // lower right
		    OUTE(1);
		}
	    } else if (connect_corners == 2) {
		if (i == 0 && j == 0) {	// upper left
		    OUTE(dim2);
		} else if (i + 1 == dim1 && j == 0) { // lower left
		    OUTE((dim1 - 1) * dim2 + dim2);
		} else if (i == 0 && j + 1 == dim2) { // upper right
		    OUTE(1);
		} else if (i + 1 == dim1 && j + 1 == dim2) { // lower right
		    OUTE((dim1 - 1) * dim2 + 1);
		}
	    }
	}
}

void makeTree(unsigned depth, unsigned nary, edgefn ef) {
    const double n = (pow(nary, depth) - 1) / (nary - 1); // no. of non-leaf nodes
    unsigned idx = 2;

    for (unsigned i = 1; i <= n; i++) {
	for (unsigned j = 0; j < nary; j++) {
	    ef (i, idx++);
	}
    }
}

void makeBinaryTree(unsigned depth, edgefn ef) {
    const unsigned n = (1u << depth) - 1;

    for (unsigned i = 1; i <= n; i++) {
	ef( i, 2 * i);
	ef( i, 2 * i + 1);
    }
}

typedef struct {
  unsigned nedges;
  unsigned *edges;
} vtx_data;

static void constructSierpinski(unsigned v1, unsigned v2, unsigned v3,
                                unsigned depth, vtx_data *graph) {
    static unsigned last_used_node_name = 3;

    if (depth > 0) {
	const unsigned v4 = ++last_used_node_name;
	const unsigned v5 = ++last_used_node_name;
	const unsigned v6 = ++last_used_node_name;
	constructSierpinski(v1, v4, v5, depth - 1, graph);
	constructSierpinski(v2, v5, v6, depth - 1, graph);
	constructSierpinski(v3, v4, v6, depth - 1, graph);
	return;
    }
    // depth==0, Construct graph:

    unsigned nedges = graph[v1].nedges;
    graph[v1].edges[nedges++] = v2;
    graph[v1].edges[nedges++] = v3;
    graph[v1].nedges = nedges;

    nedges = graph[v2].nedges;
    graph[v2].edges[nedges++] = v1;
    graph[v2].edges[nedges++] = v3;
    graph[v2].nedges = nedges;

    nedges = graph[v3].nedges;
    graph[v3].edges[nedges++] = v1;
    graph[v3].edges[nedges++] = v2;
    graph[v3].nedges = nedges;
}

void makeSierpinski(unsigned depth, edgefn ef) {
    vtx_data* graph;

    depth--;
    const unsigned n = 3 * (1 + ((unsigned)(pow(3.0, depth) + 0.5) - 1) / 2);

    graph = gv_calloc(n + 1, sizeof(vtx_data));
    unsigned *edges = gv_calloc(4 * n, sizeof(unsigned));

    for (unsigned i = 1; i <= n; i++) {
	graph[i].edges = edges;
	edges += 4;
	graph[i].nedges = 0;
    }

    constructSierpinski(1, 2, 3, depth, graph);

    for (unsigned i = 1; i <= n; i++) {
	// write the neighbors of the node i
	for (unsigned j = 0; j < graph[i].nedges; j++) {
	    const unsigned nghbr = graph[i].edges[j];
	    if (i < nghbr) ef( i, nghbr);
	}
    }

    free(graph[1].edges);
    free(graph);
}

static void constructTetrix(unsigned v1, unsigned v2, unsigned v3, unsigned v4,
                            unsigned depth, vtx_data* graph) {
    static unsigned last_used_node_name = 4;

    if (depth > 0) {
        const unsigned v5 = ++last_used_node_name;
        const unsigned v6 = ++last_used_node_name;
        const unsigned v7 = ++last_used_node_name;
        const unsigned v8 = ++last_used_node_name;
        const unsigned v9 = ++last_used_node_name;
        const unsigned v10 = ++last_used_node_name;
        constructTetrix(v1, v5, v6, v8, depth - 1, graph);
        constructTetrix(v2, v6, v7, v9, depth - 1, graph);
        constructTetrix(v3, v5, v7, v10, depth - 1, graph);
        constructTetrix(v4, v8, v9, v10, depth - 1, graph);
        return;
    }
    // depth==0, Construct graph:
    unsigned nedges = graph[v1].nedges;
    graph[v1].edges[nedges++] = v2;
    graph[v1].edges[nedges++] = v3;
    graph[v1].edges[nedges++] = v4;
    graph[v1].nedges = nedges;

    nedges = graph[v2].nedges;
    graph[v2].edges[nedges++] = v1;
    graph[v2].edges[nedges++] = v3;
    graph[v2].edges[nedges++] = v4;
    graph[v2].nedges = nedges;

    nedges = graph[v3].nedges;
    graph[v3].edges[nedges++] = v1;
    graph[v3].edges[nedges++] = v2;
    graph[v3].edges[nedges++] = v4;
    graph[v3].nedges = nedges;

    nedges = graph[v4].nedges;
    graph[v4].edges[nedges++] = v1;
    graph[v4].edges[nedges++] = v2;
    graph[v4].edges[nedges++] = v3;
    graph[v4].nedges = nedges;
}

void makeTetrix(unsigned depth, edgefn ef) {
    vtx_data* graph;

    depth--;
    const unsigned n = 4 + 2 * (((unsigned)(pow(4.0, depth) + 0.5) - 1));

    graph = gv_calloc(n + 1, sizeof(vtx_data));
    unsigned *edges = gv_calloc(6 * n, sizeof(unsigned));

    for (unsigned i = 1; i <= n; i++) {
        graph[i].edges = edges;
        edges += 6;
        graph[i].nedges = 0;
    }

    constructTetrix(1, 2, 3, 4, depth, graph);

    for (unsigned i = 1; i <= n; i++) {
        // write the neighbors of the node i
        for (unsigned j = 0; j < graph[i].nedges; j++) {
            const unsigned nghbr = graph[i].edges[j];
            if (i < nghbr) ef( i, nghbr);
        }
    }

    free(graph[1].edges);
    free(graph);
}

void makeHypercube(unsigned dim, edgefn ef) {
    const unsigned n = 1u << dim;

    for (unsigned i = 0; i < n; i++) {
	for (unsigned j = 0; j < dim; j++) {
	    const unsigned neighbor = (i ^ (1u << j)) + 1;
	    if (i < neighbor)
		ef( i + 1, neighbor);
	}
    }
}

void makeTriMesh(unsigned sz, edgefn ef) {
    if (sz == 1) {
	ef (1, 0);
	return;
    }
    ef(1,2);
    ef(1,3);
    unsigned idx = 2;
    for (unsigned i = 2; i < sz; i++) {
	for (unsigned j = 1; j <= i; j++) {
	    ef(idx,idx+i);
	    ef(idx,idx+i+1);
	    if (j < i)
		ef(idx,idx+1);
	    idx++;
	}
    }
    for (unsigned j = 1; j < sz; j++) {
	ef (idx,idx+1);
	idx++;
    }
}

void makeBall(unsigned w, unsigned h, edgefn ef) {
    makeCylinder (w, h, ef);

    for (unsigned i = 1; i <= h; i++)
	ef (0, i);

    const unsigned cap = w * h + 1;
    for (unsigned i = (w - 1) * h + 1; i <= w * h; i++)
	ef (i, cap);

}

/* makeRandom:
 * No. of nodes is largest 2^n - 1 less than or equal to h.
 */
void makeRandom(unsigned h, unsigned w, edgefn ef) {
    const int type = rand() % 2;

    unsigned size = 0;
    unsigned depth = 0;
    while (size <= h) {
	size += 1u << depth;
	depth++;
    }
    depth--;
    if (size > h) {
	size -= 1u << depth;
	depth--;
    }

    if (type)
	makeBinaryTree (depth, ef);
    else
	makePath (size, ef);

    for (unsigned i = 3; i <= size; i++) {
	for (unsigned j = 1; j + 1 < i; j++) {
	    const unsigned th = (unsigned)rand() % (size * size);
	    if ((th <= w * w && (i < 5 || (i + 4 > h && j + 4 > h))) || th <= w)
		ef(j,i);
	}
    }
}

void makeMobius(unsigned w, unsigned h, edgefn ef) {
    if (h == 1) {
	fprintf(stderr, "Warning: degenerate Moebius strip of %u vertices\n", w);
	makePath(w, ef);
	return;
    }
    if (w == 1) {
	fprintf(stderr, "Warning: degenerate Moebius strip of %u vertices\n", h);
	makePath(h, ef);
	return;
    }

    for (unsigned i = 0; i + 1 < w; i++) {
        for (unsigned j = 1; j < h; j++){
            ef(j + i*h, j + (i+1)*h);
            ef(j + i*h, j+1 + i*h);
        }
    }

    for (unsigned i = 1; i < h; i++){
        ef (i + (w-1)*h, i+1 + (w-1)*h);
    }
    for (unsigned i=1; i < w; i++) {
        ef(i*h , (i+1)*h);
        ef(i*h, (w-i)*h+1);
    }

    ef(1,w*h);
}

typedef struct {
    unsigned j, d;
} pair;

typedef struct {
    unsigned top, root;
    unsigned* p; 
} tree_t;

static tree_t *mkTree(unsigned sz) {
    tree_t* tp = gv_alloc(sizeof(tree_t));
    tp->root = 0;
    tp->top = 0;
    tp->p = gv_calloc(sz, sizeof(unsigned));
    return tp;
}

static void
freeTree (tree_t* tp)
{
    free (tp->p);
    free (tp);
}

static void
resetTree (tree_t* tp)
{
    tp->root = 0;
    tp->top = 0;
}

static unsigned treeRoot(tree_t* tp) {
    return tp->root;
}

static unsigned prevRoot(tree_t *tp) {
    return tp->p[tp->root];
}

static unsigned treeSize(tree_t *tp) {
    return tp->top - tp->root + 1;
}

static unsigned treeTop(tree_t *tp) {
    return tp->top;
}

static void
treePop (tree_t* tp)
{
    tp->root = prevRoot(tp);
}

static void addTree(tree_t *tp, unsigned sz) {
	tp->p[tp->top+1] = tp->root;
	tp->root = tp->top+1;
	tp->top += sz;
	if (sz > 1) tp->p[tp->top] = tp->top-1;
}

static void treeDup(tree_t *tp, unsigned J) {
    unsigned M = treeSize(tp);
    unsigned L = treeRoot(tp);
    unsigned LL = prevRoot(tp);
    unsigned LS = L + (J-1)*M - 1;
    for (unsigned i = L; i <= LS; i++) {
	if ((i-L)%M == 0)  tp->p[i+M] = LL;
	else tp->p[i+M] = tp->p[i] + M;
    }
    tp->top = LS + M;
}

/*****************/

DEFINE_LIST(int_stack, unsigned)

static void push(int_stack_t *sp, unsigned j, unsigned d) {
  int_stack_push_back(sp, j);
  int_stack_push_back(sp, d);
}

static pair pop(int_stack_t *sp) {

  // extract ints in the opposite order in which they were pushed
  const unsigned d = int_stack_pop_back(sp);
  const unsigned j = int_stack_pop_back(sp);

  return (pair){j, d};
}

/*****************/

/// multiply two 64-bit unsigned integers, exiting on overflow
static uint64_t umul(uint64_t a, uint64_t b) {
  uint64_t res;
  if (u64mul_overflow(a, b, &res)) {
    fprintf(stderr, "integer overflow in %" PRIu64 " * %" PRIu64 "\n", a, b);
    graphviz_exit(EXIT_FAILURE);
  }
  return res;
}

/// add two 64-bit unsigned integers, exiting on overflow
static uint64_t uadd(uint64_t a, uint64_t b) {
  uint64_t res;
  if (u64add_overflow(a, b, &res)) {
    fprintf(stderr, "integer overflow in %" PRIu64 " + %" PRIu64 "\n", a, b);
    graphviz_exit(EXIT_FAILURE);
  }
  return res;
}

static uint64_t *genCnt(unsigned NN) {
    uint64_t *T = gv_calloc(NN + 1, sizeof(uint64_t));
    unsigned NLAST = 1;
    T[1] = 1;
    while (NN > NLAST) {
	uint64_t SUM = 0;
	for (unsigned D = 1; D <= NLAST; D++) {
	    unsigned I = NLAST + 1;
	    const uint64_t TD = umul(T[D], D);
	    for (unsigned J = 1; J <= NLAST; J++) {
		if (I <= D) break;
		I = I-D;
		SUM = uadd(SUM, umul(T[I], TD));
	    }
	}
	NLAST++;
	T[NLAST] = SUM/(NLAST-1);
    }
    return T;
}

static void genTree(unsigned NN, uint64_t *T, int_stack_t *stack,
                    tree_t *TREE) {
    pair p;
    unsigned J;

    unsigned N = NN;

    while (1) {
	while (N > 2) {
	    const uint64_t v = umul(N - 1, T[N]);
	    uint64_t Z = gv_random_u64(v);
	    unsigned D = 0;
	    bool more = true;
	    unsigned M;
	    do {
		D++;
		const uint64_t TD = umul(D, T[D]);
		M = N;
		J = 0;
		do {
		    J++;
		    if (M < D + 1) break;
		    M -= D;
		    if (Z < umul(T[M], TD)) {
                      more = false;
                      break;
                    }
		    Z -= umul(T[M], TD);
		} while (true);
	    } while (more);
	    push(stack, J, D);
	    N = M;
	}
	addTree (TREE, N);
	 
	while (1) {
	    p = pop(stack);
	    N = p.d;
	    if (N != 0) {
		push(stack,p.j,0);
		break;
	    }
	    J = p.j;
	    if (J > 1) treeDup (TREE, J);
	    if (treeTop(TREE) == NN) return;
	    treePop(TREE);
	}
    }

}

static void
writeTree (tree_t* tp, edgefn ef)
{
    for (unsigned i = 2; i <= tp->top; i++)
	ef (tp->p[i], i);
}

struct treegen_s {
    unsigned N;
    uint64_t *T;
    int_stack_t sp;
    tree_t* tp;
};

treegen_t *makeTreeGen(unsigned N) {
    treegen_t* tg = gv_alloc(sizeof(treegen_t));

    tg->N = N;
    tg->T = genCnt(N);
    tg->sp = (int_stack_t){0};
    tg->tp = mkTree(N+1);

    return tg;
}

void makeRandomTree (treegen_t* tg, edgefn ef)
{
    int_stack_clear(&tg->sp);
    resetTree(tg->tp);
    genTree(tg->N, tg->T, &tg->sp, tg->tp);
    writeTree (tg->tp, ef);
}

void 
freeTreeGen(treegen_t* tg)
{
    free (tg->T);
    int_stack_free(&tg->sp);
    freeTree (tg->tp);
    free (tg);
}

