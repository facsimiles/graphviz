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

#include <assert.h>
#include <neatogen/hedges.h>
#include <common/render.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/alloc.h>
#include <util/arena.h>

#define DELETED -2

void ELcleanup(el_state_t *st) {
    gv_arena_reset(&st->allocated);
    free(st->hash);
}

void ELinitialize(el_state_t *st) {
    assert(st != NULL);

    *st = (el_state_t){0};

    st->hashsize = 2 * sqrt_nsites;
    st->hash = gv_calloc(st->hashsize, sizeof(Halfedge *));
    st->leftend = HEcreate(st, NULL, 0);
    st->rightend = HEcreate(st, NULL, 0);
    st->leftend->ELleft = NULL;
    st->leftend->ELright = st->rightend;
    st->rightend->ELleft = st->leftend;
    st->rightend->ELright = NULL;
    st->hash[0] = st->leftend;
    st->hash[st->hashsize - 1] = st->rightend;
}

Site *hintersect(Halfedge *el1, Halfedge *el2, arena_t *allocator) {
    assert(allocator != NULL);
    Edge *e1, *e2, *e;
    Halfedge *el;
    double d, xint, yint;
    bool right_of_site;

    e1 = el1->ELedge;
    e2 = el2->ELedge;
    if (e1 == NULL || e2 == NULL)
	return NULL;
    if (e1->reg[1] == e2->reg[1])
	return NULL;

    d = e1->a * e2->b - e1->b * e2->a;
    if (-1.0e-10 < d && d < 1.0e-10)
	return NULL;

    xint = (e1->c * e2->b - e2->c * e1->b) / d;
    yint = (e2->c * e1->a - e1->c * e2->a) / d;

    if (e1->reg[1]->coord.y < e2->reg[1]->coord.y ||
	(e1->reg[1]->coord.y == e2->reg[1]->coord.y &&
	 e1->reg[1]->coord.x < e2->reg[1]->coord.x)) {
	el = el1;
	e = e1;
    } else {
	el = el2;
	e = e2;
    }
    right_of_site = xint >= e->reg[1]->coord.x;
    if ((right_of_site && el->ELpm == le) || (!right_of_site && el->ELpm == re))
	return NULL;

    Site *const v = ARENA_NEW(allocator, Site);
    v->coord.x = xint;
    v->coord.y = yint;
    return v;
}

/* returns 1 if p is to right of halfedge e */
static int right_of(Halfedge *el, Point *p) {
    Edge *e;
    Site *topsite;
    int above, fast;
    double dxp, dyp, dxs, t1, t2, t3, yl;

    e = el->ELedge;
    topsite = e->reg[1];
    bool right_of_site = p->x > topsite->coord.x;
    if (right_of_site && el->ELpm == le)
	return 1;
    if (!right_of_site && el->ELpm == re)
	return 0;

    if (e->a == 1.0) {
	dyp = p->y - topsite->coord.y;
	dxp = p->x - topsite->coord.x;
	fast = 0;
	if ((!right_of_site && e->b < 0.0) || (right_of_site && e->b >= 0.0)) {
	    above = dyp >= e->b * dxp;
	    fast = above;
	} else {
	    above = p->x + p->y * e->b > e->c;
	    if (e->b < 0.0)
		above = !above;
	    if (!above)
		fast = 1;
	}
	if (!fast) {
	    dxs = topsite->coord.x - (e->reg[0])->coord.x;
	    above = e->b * (dxp * dxp - dyp * dyp) <
		dxs * dyp * (1.0 + 2.0 * dxp / dxs + e->b * e->b);
	    if (e->b < 0.0)
		above = !above;
	}
    } else {			/*e->b==1.0 */
	yl = e->c - e->a * p->x;
	t1 = p->y - yl;
	t2 = p->x - topsite->coord.x;
	t3 = yl - topsite->coord.y;
	above = t1 * t1 > t2 * t2 + t3 * t3;
    }
    return el->ELpm == le ? above : !above;
}

Halfedge *HEcreate(el_state_t *st, Edge *e, char pm) {
    Halfedge *answer = ARENA_NEW(&st->allocated, Halfedge);
    answer->ELedge = e;
    answer->ELpm = pm;
    answer->PQnext = NULL;
    answer->vertex = NULL;
    return answer;
}


void ELinsert(Halfedge * lb, Halfedge * new)
{
    new->ELleft = lb;
    new->ELright = lb->ELright;
    lb->ELright->ELleft = new;
    lb->ELright = new;
}

/* Get entry from hash table, pruning any deleted nodes */
static Halfedge *ELgethash(el_state_t *st, int b) {
    assert(st != NULL);
    Halfedge *he;

    if (b < 0 || b >= st->hashsize)
	return NULL;
    he = st->hash[b];
    if (he == NULL || he->ELedge != (Edge *) DELETED)
	return he;

/* Hash table points to deleted half edge.  Patch as necessary. */
    st->hash[b] = NULL;
    return NULL;
}

/// convert a `double` to an `int`, between bounds
///
/// @param lower Lower bound to limit to
/// @param v Value to convert
/// @param upper Upper bound to limit to
/// @return A converted value in the range [lower, upper]
static int clamp(int lower, double v, int upper) {
  assert(upper >= lower);
  if (v < lower) {
    return lower;
  }
  if (v > upper) {
    return upper;
  }
  return (int)v;
}

Halfedge *ELleftbnd(el_state_t *st, Point *p) {
    assert(st != NULL);
    int i;
    Halfedge *he;

/* Use hash table to get close to desired halfedge */
    const int bucket =
      clamp(0, (p->x - xmin) / deltax * st->hashsize, st->hashsize - 1);
    he = ELgethash(st, bucket);
    if (he == NULL) {
	for (i = 1; ; ++i) {
	    if ((he = ELgethash(st, bucket - i)) != NULL)
		break;
	    if ((he = ELgethash(st, bucket + i)) != NULL)
		break;
	}
    }
/* Now search linear list of halfedges for the corect one */
    if (he == st->leftend || (he != st->rightend && right_of(he, p))) {
	do {
	    he = he->ELright;
	} while (he != st->rightend && right_of(he, p));
	he = he->ELleft;
    } else
	do {
	    he = he->ELleft;
	} while (he != st->leftend && !right_of(he, p));

/* Update hash table */
    if (bucket > 0 && bucket < st->hashsize - 1) {
	st->hash[bucket] = he;
    }
    return he;
}


/* This delete routine can't reclaim node, since pointers from hash
   table may be present.   */
void ELdelete(Halfedge * he)
{
    he->ELleft->ELright = he->ELright;
    he->ELright->ELleft = he->ELleft;
    he->ELedge = (Edge *) DELETED;
}


Halfedge *ELright(Halfedge * he)
{
    return he->ELright;
}

Halfedge *ELleft(Halfedge * he)
{
    return he->ELleft;
}


Site *leftreg(Halfedge * he)
{
    if (he->ELedge == NULL)
	return bottomsite;
    return he->ELpm == le ? he->ELedge->reg[le] : he->ELedge->reg[re];
}

Site *rightreg(Halfedge * he)
{
    if (he->ELedge == NULL)
	return bottomsite;
    return he->ELpm == le ? he->ELedge->reg[re] : he->ELedge->reg[le];
}
