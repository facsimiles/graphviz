/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <cgraph/cghdr.h>
#include <stddef.h>

static Agraph_t *Ag_dictop_G;

void agdictobjfree(void *p, Dtdisc_t *disc) {
    Agraph_t *g;

    (void)disc;
    g = Ag_dictop_G;
    if (g)
	agfree(g, p);
    else
	free(p);
}

Dict_t *agdtopen(Agraph_t * g, Dtdisc_t * disc, Dtmethod_t * method)
{
    Dict_t *d;

    Ag_dictop_G = g;
    d = dtopen(disc, method);
    Ag_dictop_G = NULL;
    return d;
}

int agdtdelete(Agraph_t * g, Dict_t * dict, void *obj)
{
    Ag_dictop_G = g;
    return dtdelete(dict, obj) != NULL;
}

int agdtclose(Agraph_t * g, Dict_t * dict)
{
    dtdisc(dict, NULL);
    Ag_dictop_G = g;
    if (dtclose(dict))
	return 1;
    Ag_dictop_G = NULL;
    return 0;
}

void agdtdisc(Agraph_t * g, Dict_t * dict, Dtdisc_t * disc)
{
    (void)g; /* unused */
    if (disc && dtdisc(dict, NULL) != disc) {
	dtdisc(dict, disc);
    }
    /* else unchanged, disc is same as old disc */
}
