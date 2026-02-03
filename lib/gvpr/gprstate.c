/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/


/*
 * gpr state
 *
 */

#include "config.h"

#include <cgraph/cgraph.h>
#include <gvpr/gprstate.h>
#include <ast/error.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <util/alloc.h>
#include <util/list.h>

static int name_used;

bool validTVT(long long c) {
  return TV_flat <= c && c <= TV_prepostrev;
}

void initGPRState(Gpr_t *state) {
  state->tgtname = strdup("gvpr_result");
}

Gpr_t *openGPRState(gpr_info* info)
{
    Gpr_t *state;

    if (!(state = calloc(1, sizeof(Gpr_t)))) {
	error(ERROR_ERROR, "Could not create gvpr state: out of memory");
	return state;
    }

    state->tvt = TV_flat;
    state->name_used = name_used;
    state->tvroot = 0;
    state->tvnext = 0;
    state->tvedge = 0;
    state->outFile = info->outFile;
    state->args = info->args;
    state->errf = info->errf;
    state->flags = info->flags;

    return state;
}


static int
bindingcmpf (const void *key, const void *ip)
{
    return strcmp (((const gvprbinding*)key)->name, ((const gvprbinding*)ip)->name);
}

/* findBinding:
 */
gvprbinding* 
findBinding (Gpr_t* state, char* fname)
{
    if (!state->bindings) {
	error(ERROR_ERROR,"call(\"%s\") failed: no bindings", fname);
	return NULL;
    }
    if (!fname) {
	error(ERROR_ERROR,"NULL function name for call()");
	return NULL;
    }

    const gvprbinding key = {.name = fname};
    gvprbinding *bp = bsearch(&key, state->bindings, state->n_bindings,
                              sizeof(gvprbinding), bindingcmpf);
    if (!bp)
	error(ERROR_ERROR, "No binding for \"%s\" in call()", fname);
    return bp;
}

/* addBindings:
 * Validate input, sort lexicographically, and attach
 */
void addBindings (Gpr_t* state, gvprbinding* bindings)
{
    size_t n = 0;
    gvprbinding* bp = bindings;
    gvprbinding* buf;
    gvprbinding* bufp;

    while (bp && bp->name) {
	if (bp->fn) n++;
	bp++;
    }

    if (n == 0) return;
    bufp = buf = gv_calloc(n, sizeof(gvprbinding));
    bp = bindings;
    while (bp->name) {
        if (bp->fn) {
	    *bufp = *bp;
	    bufp++;
	}
	bp++;
    }
    qsort (buf, n, sizeof(gvprbinding), bindingcmpf);

    state->bindings = buf;
    state->n_bindings = n;
}

void closeGPRState(Gpr_t* state)
{
    if (!state) return;

    // we manually delete the managed graphs rather than setting
    // `state->open_graphs.dtor` in `openGPRState` to allow list items to be
    // removed in e.g. `getval` without closing the graph
    for (size_t i = 0; i < LIST_SIZE(&state->open_graphs); ++i) {
	(void)agclose(LIST_GET(&state->open_graphs, i));
    }
    LIST_FREE(&state->open_graphs);

    name_used = state->name_used;
    free(state->tgtname);
    free (state->dp);
    free (state);
}
