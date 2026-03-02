/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include "render.h"
#include <fdpgen/fdp.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
	graph_t *parent;
    } rdata;

#define RDATA(n) ((rdata*)(ND_alg(n)))
#define SPARENT(n) (RDATA(n)->parent)

extern void patchwork_layout(Agraph_t * g);
extern void patchwork_cleanup(Agraph_t * g);
extern void patchworkLayout(Agraph_t *g);

#ifdef __cplusplus
}
#endif
