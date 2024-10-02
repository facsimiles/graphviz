/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sfio/sfio.h>
#include <parse.h>
#include <gprstate.h>
#include <expr/expr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

    typedef struct {
	Exnode_t *guard;
	Exnode_t *action;
    } case_stmt;

#define UDATA "userval"

    typedef struct {
	Agrec_t h;
	Extype_t iu;
	Agedge_t* ine;
    } nval_t;

typedef struct {
  bool locked: 1; ///< is the lock currently taken?
  bool zombie: 1; ///< was a deletion request recorded while locked?
} lock_t;

    typedef struct {
	Agrec_t h;
	lock_t lock;
    } gval_t;

    typedef struct {
	Agrec_t h;
    } edata;

#define OBJ(p) ((Agobj_t*)p)

    typedef nval_t ndata;
    typedef gval_t gdata;

#define nData(n)    ((ndata*)(aggetrec(n,UDATA,0)))
#define gData(g)    ((gdata*)(aggetrec(g,UDATA,0)))

typedef struct {
  bool srcout: 1;
  bool induce: 1;
  bool clone: 1;
} compflags_t;

    typedef struct {
	Exnode_t *begg_stmt;
	bool does_walk_graph; ///< does this block have a node or edge statement?
	size_t n_nstmts;
	size_t n_estmts;
	case_stmt *node_stmts;
	case_stmt *edge_stmts;
    } comp_block; 

    typedef struct {
	bool uses_graph; ///< does this program use the input graph?
	Expr_t *prog;
	Exnode_t *begin_stmt;
	size_t n_blocks;
	comp_block  *blocks;
	Exnode_t *endg_stmt;
	Exnode_t *end_stmt;
    } comp_prog;

comp_prog *compileProg(parse_prog *, Gpr_t *, compflags_t);
    extern void freeCompileProg (comp_prog *p);
    extern Agraph_t *readG(FILE *fp);
    extern Agraph_t *openG(char *name, Agdesc_t);
    extern Agraph_t *openSubg(Agraph_t * g, char *name);
    extern Agnode_t *openNode(Agraph_t * g, char *name);
    extern Agedge_t *openEdge(Agraph_t* g, Agnode_t * t, Agnode_t * h, char *key);

#ifdef __cplusplus
}
#endif
