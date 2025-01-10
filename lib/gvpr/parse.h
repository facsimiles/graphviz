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

#include <stdlib.h>
#include <util/list.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum { Begin =
	    0, End, BeginG, EndG, Node, Edge, Eof, Error } case_t;

    typedef struct {
	int gstart;
	char *guard;
	int astart;
	char *action;
    } case_info;

static inline void free_case_info(case_info c) {
  free(c.guard);
  free(c.action);
}

DEFINE_LIST_WITH_DTOR(case_infos, case_info, free_case_info)

    typedef struct {
	int l_beging;
	char *begg_stmt;
	case_infos_t node_stmts;
	case_infos_t edge_stmts;
    } parse_block; 

DEFINE_LIST(parse_blocks, parse_block)

    typedef struct {
	char *source;
	int l_begin, l_end, l_endg;
	char *begin_stmt;
	parse_blocks_t blocks;
	char *endg_stmt;
	char *end_stmt;
    } parse_prog;

    extern parse_prog *parseProg(char *, int);
    extern void freeParseProg (parse_prog *);

#ifdef __cplusplus
}
#endif
