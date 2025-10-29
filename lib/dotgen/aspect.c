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
#include <dotgen/dot.h>
#include <stddef.h>
#include <util/alloc.h>
#include <util/bitarray.h>

/*
 * Author: Mohammad T. Irfan
 *   Summer, 2008
 */

/* TODO:
 *   - Support clusters
 *   - Support disconnected graphs
 *   - Provide algorithms for aspect ratios < 1
 */

#define DEF_PASSES 5

void setAspect(Agraph_t *g) {
    double rv;
    char *p;
    int passes = DEF_PASSES;

    p = agget (g, "aspect");

    if (!p || sscanf (p, "%lf,%d", &rv, &passes) <= 0) {
	return;
    }
    agwarningf("the aspect attribute has been disabled due to implementation flaws - attribute ignored.\n");
}
