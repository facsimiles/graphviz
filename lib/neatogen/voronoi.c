/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <neatogen/mem.h>
#include <neatogen/geometry.h>
#include <neatogen/edges.h>
#include <neatogen/hedges.h>
#include <neatogen/heap.h>
#include <neatogen/voronoi.h>
#include <util/gv_math.h>
#include <util/list.h>

void voronoi(Site *(*nextsite)(void *context), void *context) {
    Site *newsite, *bot, *top, *p;
    Site *v;
    Point newintstar = {0};
    char pm;
    Halfedge *lbnd, *rbnd, *llbnd, *rrbnd, *bisector;
    Edge *e;

    LIST(Edge *) allocated = {0}; // live edges to be freed
    siteinit();
    pq_t *pq = PQinitialize();
    bottomsite = nextsite(context);
    el_state_t st = {0};
    ELinitialize(&st);

    newsite = nextsite(context);
    while (1) {
	if (!PQempty(pq))
	    newintstar = PQ_min(pq);

	if (newsite != NULL &&
      (PQempty(pq) || newsite->coord.y < newintstar.y ||
       (newsite->coord.y ==newintstar.y && newsite->coord.x < newintstar.x))) {
	    /* new site is smallest */
	    lbnd = ELleftbnd(&st, &newsite->coord);
	    rbnd = ELright(lbnd);
	    bot = rightreg(lbnd);
	    e = gvbisect(bot, newsite);
	    LIST_APPEND(&allocated, e);
	    bisector = HEcreate(&st, e, le);
	    ELinsert(lbnd, bisector);
	    if ((p = hintersect(lbnd, bisector)) != NULL) {
		PQdelete(pq, lbnd);
		PQinsert(pq, lbnd, p, ngdist(p, newsite));
	    }
	    lbnd = bisector;
	    bisector = HEcreate(&st, e, re);
	    ELinsert(lbnd, bisector);
	    if ((p = hintersect(bisector, rbnd)) != NULL)
		PQinsert(pq, bisector, p, ngdist(p, newsite));
	    newsite = nextsite(context);
	} else if (!PQempty(pq)) {
	    /* intersection is smallest */
	    lbnd = PQextractmin(pq);
	    llbnd = ELleft(lbnd);
	    rbnd = ELright(lbnd);
	    rrbnd = ELright(rbnd);
	    bot = leftreg(lbnd);
	    top = rightreg(rbnd);
	    v = lbnd->vertex;
	    makevertex(v);
	    if (endpoint(lbnd->ELedge, lbnd->ELpm, v)) {
		LIST_REMOVE(&allocated, lbnd->ELedge);
	    }
	    if (endpoint(rbnd->ELedge, rbnd->ELpm, v)) {
		LIST_REMOVE(&allocated, rbnd->ELedge);
	    }
	    ELdelete(lbnd);
	    PQdelete(pq, rbnd);
	    ELdelete(rbnd);
	    pm = le;
	    if (bot->coord.y > top->coord.y) {
		SWAP(&bot, &top);
		pm = re;
	    }
	    e = gvbisect(bot, top);
	    LIST_APPEND(&allocated, e);
	    bisector = HEcreate(&st, e, pm);
	    ELinsert(llbnd, bisector);
	    if (endpoint(e, re - pm, v)) {
		LIST_REMOVE(&allocated, e);
	    }
	    deref(v);
	    if ((p = hintersect(llbnd, bisector)) != NULL) {
		PQdelete(pq, llbnd);
		PQinsert(pq, llbnd, p, ngdist(p, bot));
	    }
	    if ((p = hintersect(bisector, rrbnd)) != NULL) {
		PQinsert(pq, bisector, p, ngdist(p, bot));
	    }
	} else
	    break;
    }

    for (lbnd = ELright(st.leftend); lbnd != st.rightend;
         lbnd = ELright(lbnd)) {
	e = lbnd->ELedge;
	clip_line(e);
    }

    ELcleanup(&st);

    // `PQcleanup` relies on the number of sites, so should be discarded and
    // at least every time we use `vAdjust`. See note in adjust.c:cleanup().
    PQcleanup(pq);

    for (size_t i = 0; i < LIST_SIZE(&allocated); ++i) {
	free(LIST_GET(&allocated, i));
    }
    LIST_FREE(&allocated);
}
