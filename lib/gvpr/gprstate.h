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
#include "cgraph.h"
#include <ast/ast.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <expr/expr.h>
#include <util/list.h>
#include <util/strview.h>
#include <util/list.h>
#include "gvpr.h"

typedef LIST(strview_t) strviews_t;

    typedef enum { TV_flat, TV_ne, TV_en, 
                   TV_bfs, 
                   TV_dfs, TV_fwd, TV_rev,
                   TV_postdfs, TV_postfwd, TV_postrev,
                   TV_prepostdfs, TV_prepostfwd, TV_prepostrev,
    } trav_type;

    typedef struct {
	Agraph_t *curgraph;
	Agraph_t *nextgraph;
	Agraph_t *target;
	Agraph_t *outgraph;
	Agobj_t *curobj;
	Exdisc_t *dp;
	Exerror_f errf;
	Exexit_f exitf;
	char *tgtname;
	char *infname;
	FILE *outFile;
	trav_type tvt;
	Agnode_t *tvroot;
	Agnode_t *tvnext;
	Agedge_t *tvedge;
	int name_used;
	strviews_t args;
	int flags;
	gvprbinding* bindings;
	size_t n_bindings;
	jmp_buf jbuf;
	LIST(Agraph_t *) open_graphs;
    } Gpr_t;

    typedef struct {
	FILE *outFile;
	strviews_t args;
	Exerror_f errf;
	Exexit_f exitf;
	int flags;
    } gpr_info;

    extern Gpr_t *openGPRState(gpr_info*);
    extern void addBindings(Gpr_t* state, gvprbinding*);
    extern gvprbinding* findBinding(Gpr_t* state, char*);
    extern void closeGPRState(Gpr_t* state);
    extern void initGPRState(Gpr_t *);
    extern bool validTVT(long long);

#ifdef __cplusplus
}
#endif
