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
 * Tcl binding to drive Stephen North's and
 * Emden Gansner's shortest path code.
 *
 * ellson@graphviz.org   October 2nd, 1996
 */

#include "config.h"

#include "../tcl-compat.h"
#include                <sys/types.h>
#include                <stdbool.h>
#include                <stdint.h>
#include                <stdlib.h>
#include                <string.h>

#include <assert.h>
#include <limits.h>
#include "makecw.h"
#include <math.h>
#include <pathplan/pathutil.h>
#include <pathplan/vispath.h>
#include <pathplan/tri.h>
#include "Plegal_arrangement.h"
#include <tcl.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/itos.h>
#include <util/list.h>
#include <util/prisize_t.h>

#if (TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION >= 6) || TCL_MAJOR_VERSION > 8
#else
#ifndef Tcl_GetStringResult
#define Tcl_GetStringResult(interp) interp->result
#endif
#endif

typedef Ppoint_t point;

typedef struct poly_s {
    int id;
    Ppoly_t boundary;
} poly;

/// printf format for TCL handles
#define HANDLE_FORMAT ("vgpane%" PRISIZE_T)

typedef struct {
    LIST(poly) poly; // set of polygons
    vconfig_t *vc;		/* visibility graph handle */
    Tcl_Interp *interp;		/* interpreter that owns the binding */
    char *triangle_cmd;		/* why is this here any more */
    size_t index; ///< position within allocated handles list
    bool valid; ///< is this pane currently allocated?
} vgpane_t;

typedef LIST(vgpane_t) vgpanes_t;

static vgpanes_t vgpaneTable;

static int polyid = 0;		/* unique and unchanging id for each poly */

static poly *allocpoly(vgpane_t *vgp, int id, Tcl_Size npts) {
    LIST_APPEND(&vgp->poly, (poly){.id = id});
    poly *rv = LIST_BACK(&vgp->poly);
    rv->boundary.pn = 0;
    rv->boundary.ps = gv_calloc(npts, sizeof(point));
    return rv;
}

static void vc_stale(vgpane_t * vgp)
{
    if (vgp->vc) {
	Pobsclose(vgp->vc);
	vgp->vc = NULL;
    }
}

static int vc_refresh(vgpane_t * vgp)
{
    if (vgp->vc == NULL) {
	Ppoly_t **obs = gv_calloc(LIST_SIZE(&vgp->poly), sizeof(Ppoly_t*));
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++)
	    obs[i] = &LIST_AT(&vgp->poly, i)->boundary;
	if (!Plegal_arrangement(obs, LIST_SIZE(&vgp->poly)))
	    fprintf(stderr, "bad arrangement\n");
	else
	    vgp->vc = Pobsopen(obs, (int)LIST_SIZE(&vgp->poly));
	free(obs);
    }
    return vgp->vc != NULL;
}

static void dgsprintxy(agxbuf *result, int npts, const point p[]) {
    int i;

    assert(npts > 1);
    if (agxblen(result) > 0) {
	agxbputc(result, ' ');
    }
    agxbputc(result, '{');
    const char *separator = "";
    for (i = 0; i < npts; i++) {
	agxbprint(result, "%s%g %g", separator, p[i].x, p[i].y);
	separator = " ";
    }
    agxbputc(result, '}');
}

/// @param interp Interpreter context
/// @param before Command with percent expressions
/// @param vgcanvasHandle Index to use in "%r" substitution
/// @param npts Number of coordinates
/// @param ppos Coordinates to substitute for %t
static void expandPercentsEval(Tcl_Interp *interp, char *before,
                               size_t vgcanvasHandle, int npts,
                               const point *ppos) {
    agxbuf scripts = {0};

    while (1) {
	/*
	 * Find everything up to the next % character and append it to the
	 * result string.
	 */

	char *string = strchr(before, '%');
	if (string != NULL) {
	    agxbput_n(&scripts, before, (size_t)(string - before));
	    before = string;
	}
	if (*before == 0) {
	    break;
	}
	/*
	 * There's a percent sequence here.  Process it.
	 */

	switch (before[1]) {
	case 'r':
	    agxbprint(&scripts, HANDLE_FORMAT, vgcanvasHandle);
	    break;
	case 't':
	    dgsprintxy(&scripts, npts, ppos);
	    break;
	default:
	    agxbputc(&scripts, before[1]);
	    break;
	}
	if (before[1] == '\0') {
	    break;
	}
	before += 2;
    }
    const char *script_value = agxbuse(&scripts);
    if (Tcl_GlobalEval(interp, script_value) != TCL_OK)
	fprintf(stderr, "%s while in binding: %s\n\n",
		Tcl_GetStringResult(interp), script_value);
    agxbfree(&scripts);
}

static void triangle_callback(void *vgparg, const point pqr[]) {
    vgpane_t *vgp = vgparg;
    if (vgp->triangle_cmd) {
	const size_t vgcanvasHandle = vgp->index;
	expandPercentsEval(vgp->interp, vgp->triangle_cmd, vgcanvasHandle, 3, pqr);
    }
}

static char *buildBindings(char *s1, const char *s2)
/*
 * previous binding in s1 binding to be added in s2 result in s3
 *
 * if s2 begins with + then append (separated by \n) else s2 replaces if
 * resultant string is null then bindings are deleted
 */
{
    char *s3;
    size_t l;

    if (s2[0] == '+') {
	if (s1) {
	    l = strlen(s2) - 1;
	    if (l) {
		agxbuf new = {0};
		agxbprint(&new, "%s\n%s", s1, s2 + 1);
		free(s1);
		return agxbdisown(&new);
	    } else {
		s3 = s1;
	    }
	} else {
	    l = strlen(s2) - 1;
	    if (l) {
		s3 = gv_strdup(s2 + 1);
	    } else {
		s3 = NULL;
	    }
	}
    } else {
	free(s1);
	l = strlen(s2);
	if (l) {
	    s3 = gv_strdup(s2);
	} else {
	    s3 = NULL;
	}
    }
    return s3;
}



/* convert x and y string args to point */
static int scanpoint(Tcl_Interp * interp, const char *argv[], point * p)
{
    if (sscanf(argv[0], "%lg", &p->x) != 1) {
	Tcl_AppendResult(interp, "invalid x coordinate: \"", argv[0], "\"", NULL);
	return TCL_ERROR;
    }
    if (sscanf(argv[1], "%lg", &p->y) != 1) {
	Tcl_AppendResult(interp, "invalid y coordinate: \"", argv[1], "\"", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

static point center(point vertex[], size_t n) {
    point c;

    c.x = c.y = 0;
    for (size_t i = 0; i < n; i++) {
	c.x += vertex[i].x;
	c.y += vertex[i].y;
    }
    c.x /= (int)n;
    c.y /= (int)n;
    return c;
}

static double distance(point p, point q)
{
    double dx, dy;

    dx = p.x - q.x;
    dy = p.y - q.y;
    return hypot(dx, dy);
}

static point rotate(point c, point p, double alpha)
{
    point q;
    double beta, r;

    r = distance(c, p);
    beta = atan2(p.x - c.x, p.y - c.y);
    const double sina = sin(beta + alpha);
    const double cosa = cos(beta + alpha);
    q.x = c.x + r * sina;
    q.y = c.y - r * cosa;	/* adjust for tk y-down */
    return q;
}

static point scale(point c, point p, double gain)
{
    point q;

    q.x = c.x + gain * (p.x - c.x);
    q.y = c.y + gain * (p.y - c.y);
    return q;
}

static bool remove_poly(vgpane_t *vgp, int id) {
    for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	if (LIST_GET(&vgp->poly, i).id == id) {
	    free(LIST_GET(&vgp->poly, i).boundary.ps);
	    for (size_t j = i++; i < LIST_SIZE(&vgp->poly); i++, j++) {
		LIST_SET(&vgp->poly, j, LIST_GET(&vgp->poly, i));
	    }
	    LIST_DROP_BACK(&vgp->poly);
	    vc_stale(vgp);
	    return true;
	}
    }
    return false;
}

static int insert_poly(Tcl_Interp *interp, vgpane_t *vgp, int id,
                       const char *vargv[], Tcl_Size vargc) {
    poly *np;
    int result;

    np = allocpoly(vgp, id, vargc);
    for (Tcl_Size i = 0; i < vargc; i += 2) {
	result = scanpoint(interp, &vargv[i], &np->boundary.ps[np->boundary.pn]);
	if (result != TCL_OK)
	    return result;
	np->boundary.pn++;
    }
    make_CW(&np->boundary);
    vc_stale(vgp);
    return TCL_OK;
}

static void make_barriers(vgpane_t *vgp, int pp, int qp, Pedge_t **barriers,
                          size_t *n_barriers) {

    size_t n = 0;
    for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	if (LIST_GET(&vgp->poly, i).id == pp)
	    continue;
	if (LIST_GET(&vgp->poly, i).id == qp)
	    continue;
	n += LIST_GET(&vgp->poly, i).boundary.pn;
    }
    Pedge_t *bar = gv_calloc(n, sizeof(Pedge_t));
    size_t b = 0;
    for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	if (LIST_GET(&vgp->poly, i).id == pp)
	    continue;
	if (LIST_GET(&vgp->poly, i).id == qp)
	    continue;
	for (size_t j = 0; j < LIST_GET(&vgp->poly, i).boundary.pn; j++) {
	    size_t k = j + 1;
	    if (k >= LIST_GET(&vgp->poly, i).boundary.pn)
		k = 0;
	    bar[b].a = LIST_GET(&vgp->poly, i).boundary.ps[j];
	    bar[b].b = LIST_GET(&vgp->poly, i).boundary.ps[k];
	    b++;
	}
    }
    assert(b == n);
    *barriers = bar;
    *n_barriers = n;
}

/* append the x and y coordinates of a point to the Tcl result */
static void appendpoint(Tcl_Interp * interp, point p)
{
    char buf[30];

    snprintf(buf, sizeof(buf), "%g", p.x);
    Tcl_AppendElement(interp, buf);
    snprintf(buf, sizeof(buf), "%g", p.y);
    Tcl_AppendElement(interp, buf);
}

/// lookup an existing pane
///
/// @param panes Collection to search
/// @param handle Text descriptor of the pane
/// @return A pointer to the pane or `NULL` if there is no such pane
static vgpane_t *find_vgpane(vgpanes_t *panes, const char *handle) {
  assert(panes != NULL);
  assert(handle != NULL);

  size_t index;
  if (sscanf(handle, HANDLE_FORMAT, &index) != 1) {
    return NULL;
  }
  if (index >= LIST_SIZE(panes)) {
    return NULL;
  }
  vgpane_t *const pane = LIST_AT(panes, index);
  if (!pane->valid) {
    return NULL;
  }

  return pane;
}

/// cleanup unused panes where possible
///
/// Panes need to have a stable identifier (index) for their lifetime. Thus they
/// cannot be removed from a `vgpanes_t` when deallocated because removing them
/// would alter the index of any still-live panes that comes after them.
/// Instead, panes are marked `->valid = false` when deallocated and then later
/// removed here.
///
/// It is only safe to remove deallocated panes that have no live panes
/// following them, which is exactly what this function does.
///
/// @param panes Collection to sweep
static void garbage_collect_vgpanes(vgpanes_t *panes) {
  assert(panes != NULL);

  // shrink list, discarding previously deallocated entries
  while (!LIST_IS_EMPTY(panes)) {
    if (!LIST_BACK(panes)->valid) {
      LIST_DROP_BACK(panes);
    }
  }

  // if this left us with none, fully deallocate to make leak checkers happy
  if (LIST_IS_EMPTY(panes)) {
    LIST_FREE(panes);
  }
}

/* process vgpane methods */
static int
vgpanecmd(ClientData clientData, Tcl_Interp * interp, int argc,
	  const char *argv[])
{
    (void)clientData;

    int result;
    vgpane_t *vgp;
    point p, q, *ps;
    double alpha, gain;
    Pvector_t slopes[2];
    Ppolyline_t line, spline;
    Pedge_t *barriers;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 " ", argv[0], " method ?arg arg ...?\"", NULL);
	return TCL_ERROR;
    }
    if (!(vgp = find_vgpane(&vgpaneTable, argv[0]))) {
	Tcl_AppendResult(interp, "Invalid handle: \"", argv[0], "\"", NULL);
	return TCL_ERROR;
    }

    if (strcmp(argv[1], "coords") == 0) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id ?x1 y1 x2 y2...?\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    /* find poly and return its coordinates */
	    for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
		if (LIST_GET(&vgp->poly, i).id == polyid) {
		    const size_t n = LIST_GET(&vgp->poly, i).boundary.pn;
		    for (size_t j = 0; j < n; j++) {
			appendpoint(interp, LIST_GET(&vgp->poly, i).boundary.ps[j]);
		    }
		    return TCL_OK;
		}
	    }
	    Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	/* accept either inline or delimited list */
	Tcl_Size vargc;
	const char **vargv;
	bool vargv_needs_free = false;
	if (argc == 4) {
	    result = Tcl_SplitList(interp, argv[3], &vargc, &vargv);
	    if (result != TCL_OK) {
		return result;
	    }
	    vargv_needs_free = true;
	} else {
	    vargc = (Tcl_Size)argc - 3;
	    vargv = &argv[3];
	}
	if (!vargc || vargc % 2) {
	    Tcl_AppendResult(interp,
			     "There must be a multiple of two terms in the list.", NULL);
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return TCL_ERROR;
	}

	/* remove old poly, add modified polygon to the end with 
	   the same id as the original */

	if (!remove_poly(vgp, polyid)) {
	    Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return TCL_ERROR;
	}

	result = insert_poly(interp, vgp, polyid, vargv, vargc);
	if (vargv_needs_free) {
	    Tcl_Free((char *)vargv);
	}
	return result;

    } else if (strcmp(argv[1], "debug") == 0) {
	/* debug only */
	printf("debug output goes here\n");
	return TCL_OK;

    } else if (strcmp(argv[1], "delete") == 0) {
	/* delete a vgpane and all memory associated with it */
	free(vgp->triangle_cmd);
	if (vgp->vc)
	    Pobsclose(vgp->vc);
	LIST_FREE(&vgp->poly);
	Tcl_DeleteCommand(interp, argv[0]);
	vgp->valid = false;
	garbage_collect_vgpanes(&vgpaneTable);
	return TCL_OK;

    } else if (strcmp(argv[1], "find") == 0) {
	/* find the polygon that the point is inside and return it
	   id, or null */
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " x y\"", NULL);
	    return TCL_ERROR;
	}
	const char **vargv;
	bool vargv_needs_free = false;
	if (argc == 3) {
	    result = Tcl_SplitList(interp, argv[2], &(Tcl_Size){0}, &vargv);
	    if (result != TCL_OK) {
		return result;
	    }
	    vargv_needs_free = true;
	} else {
	    vargv = &argv[2];
	}
	result = scanpoint(interp, &vargv[0], &p);
	if (vargv_needs_free) {
	    Tcl_Free((char *)vargv);
	}
	if (result != TCL_OK)
	    return result;

	/* determine the polygons (if any) that contain the point */
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (in_poly(LIST_GET(&vgp->poly, i).boundary, p)) {
		Tcl_AppendElement(interp, ITOS(LIST_GET(&vgp->poly, i).id));
	    }
	}
	return TCL_OK;

    } else if (strcmp(argv[1], "insert") == 0) {
	/* add poly to end poly list, and it coordinates to the end of 
	   the point list */
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " x1 y1 x2 y2 ...\"", NULL);
	    return TCL_ERROR;
	}
	/* accept either inline or delimited list */
	Tcl_Size vargc;
	const char **vargv;
	bool vargv_needs_free = false;
	if (argc == 3) {
	    result = Tcl_SplitList(interp, argv[2], &vargc, &vargv);
	    if (result != TCL_OK) {
		return result;
	    }
	    vargv_needs_free = true;
	} else {
	    vargc = (Tcl_Size)argc - 2;
	    vargv = &argv[2];
	}

	if (!vargc || vargc % 2) {
	    Tcl_AppendResult(interp,
			     "There must be a multiple of two terms in the list.", NULL);
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return TCL_ERROR;
	}

	polyid++;

	result = insert_poly(interp, vgp, polyid, vargv, vargc);
	if (vargv_needs_free) {
	    Tcl_Free((char *)vargv);
	}
	if (result != TCL_OK)
	    return result;

	Tcl_AppendResult(interp, ITOS(polyid), NULL);
	return TCL_OK;

    } else if (strcmp(argv[1], "list") == 0) {
	/* return list of polygon ids */
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    Tcl_AppendElement(interp, ITOS(LIST_GET(&vgp->poly, i).id));
	}
	return TCL_OK;

    } else if (strcmp(argv[1], "path") == 0) {
	/* return a list of points corresponding to the shortest path
	   that does not cross the remaining "visible" polygons. */
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " x1 y1 x2 y2\"", NULL);
	    return TCL_ERROR;
	}
	Tcl_Size vargc;
	const char **vargv;
	bool vargv_needs_free = false;
	if (argc == 3) {
	    result = Tcl_SplitList(interp, argv[2], &vargc, &vargv);
	    if (result != TCL_OK) {
		return result;
	    }
	    vargv_needs_free = true;
	} else {
	    vargc = (Tcl_Size)argc - 2;
	    vargv = &argv[2];
	}
	if (vargc < 4) {
	    Tcl_AppendResult(interp,
			     "invalid points: should be: \"x1 y1 x2 y2\"", NULL);
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return TCL_ERROR;
	}
	result = scanpoint(interp, &vargv[0], &p);
	if (result != TCL_OK) {
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return result;
	}
	result = scanpoint(interp, &vargv[2], &q);
	if (vargv_needs_free) {
	    Tcl_Free((char *)vargv);
	}
	if (result != TCL_OK)
	    return result;

	/* only recompute the visibility graph if we have to */
	if (vc_refresh(vgp)) {
	    Pobspath(vgp->vc, p, POLYID_UNKNOWN, q, POLYID_UNKNOWN, &line);

	    for (size_t i = 0; i < line.pn; i++) {
		appendpoint(interp, line.ps[i]);
	    }
	}

	return TCL_OK;

    } else if (strcmp(argv[1], "bind") == 0) {
	if (argc < 2 || argc > 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     argv[0], " bind triangle ?command?\"", NULL);
	    return TCL_ERROR;
	}
	if (argc == 2) {
	    Tcl_AppendElement(interp, "triangle");
	    return TCL_OK;
	}
	char *s = NULL;
	if (strcmp(argv[2], "triangle") == 0) {
	    s = vgp->triangle_cmd;
	    if (argc == 4)
		vgp->triangle_cmd = buildBindings(s, argv[3]);
	} else {
	    Tcl_AppendResult(interp, "unknown event \"", argv[2],
			     "\": must be one of:\n\ttriangle.", NULL);
	    return TCL_ERROR;
	}
	if (argc == 3)
	    Tcl_AppendResult(interp, s, NULL);
	return TCL_OK;

    } else if (strcmp(argv[1], "bpath") == 0) {
	/* return a list of points corresponding to the shortest path
	   that does not cross the remaining "visible" polygons. */
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " x1 y1 x2 y2\"", NULL);
	    return TCL_ERROR;
	}
	Tcl_Size vargc;
	const char **vargv;
	bool vargv_needs_free = false;
	if (argc == 3) {
	    result = Tcl_SplitList(interp, argv[2], &vargc, &vargv);
	    if (result != TCL_OK) {
		return result;
	    }
	    vargv_needs_free = true;
	} else {
	    vargc = (Tcl_Size)argc - 2;
	    vargv = &argv[2];
	}
	if (vargc < 4) {
	    Tcl_AppendResult(interp,
			     "invalid points: should be: \"x1 y1 x2 y2\"", NULL);
	    return TCL_ERROR;
	}

	result = scanpoint(interp, &vargv[0], &p);
	if (result != TCL_OK) {
	    if (vargv_needs_free) {
		Tcl_Free((char *)vargv);
	    }
	    return result;
	}
	result = scanpoint(interp, &vargv[2], &q);
	if (vargv_needs_free) {
	    Tcl_Free((char *)vargv);
	}
	if (result != TCL_OK)
	    return result;

	/* determine the polygons (if any) that contain the endpoints */
	int pp = POLYID_NONE;
	int qp = POLYID_NONE;
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    const poly tpp = LIST_GET(&vgp->poly, i);
	    if (pp == POLYID_NONE && in_poly(tpp.boundary, p))
		pp = (int)i;
	    if (qp == POLYID_NONE && in_poly(tpp.boundary, q))
		qp = (int)i;
	}

	if (vc_refresh(vgp)) {
	    Pobspath(vgp->vc, p, POLYID_UNKNOWN, q, POLYID_UNKNOWN, &line);
	    size_t n_barriers;
	    make_barriers(vgp, pp, qp, &barriers, &n_barriers);
	    slopes[0].x = slopes[0].y = 0.0;
	    slopes[1].x = slopes[1].y = 0.0;
	    Proutespline(barriers, n_barriers, line, slopes, &spline);

	    for (size_t i = 0; i < spline.pn; i++) {
		appendpoint(interp, spline.ps[i]);
	    }
	}
	return TCL_OK;

    } else if (strcmp(argv[1], "bbox") == 0) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (LIST_GET(&vgp->poly, i).id == polyid) {
		Ppoly_t pp = LIST_GET(&vgp->poly, i).boundary;
		point LL, UR;
		LL = UR = pp.ps[0];
		for (size_t j = 1; j < pp.pn; j++) {
		    p = pp.ps[j];
		    UR.x = fmax(UR.x, p.x);
		    UR.y = fmax(UR.y, p.y);
		    LL.x = fmin(LL.x, p.x);
		    LL.y = fmin(LL.y, p.y);
		}
		appendpoint(interp, LL);
		appendpoint(interp, UR);
		return TCL_OK;
	    }
	}
	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;

    } else if (strcmp(argv[1], "center") == 0) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (LIST_GET(&vgp->poly, i).id == polyid) {
		appendpoint(interp, center(LIST_GET(&vgp->poly, i).boundary.ps,
					   LIST_GET(&vgp->poly, i).boundary.pn));
		return TCL_OK;
	    }
	}
	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;

    } else if (strcmp(argv[1], "triangulate") == 0) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id\"", NULL);
	    return TCL_ERROR;
	}

	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}

	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (LIST_GET(&vgp->poly, i).id == polyid) {
		Ppoly_t *polygon = &LIST_AT(&vgp->poly, i)->boundary;
		if (polygon->pn < 3) {
		    Tcl_AppendResult(interp, "polygon ", argv[2], " has fewer than 3 points "
		                     "and thus cannot be triangulated", NULL);
		    return TCL_ERROR;
		}
		Ptriangulate(polygon, triangle_callback, vgp);
		return TCL_OK;
	    }
	}
	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;
    } else if (strcmp(argv[1], "rotate") == 0) {
	if (argc < 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id alpha\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[3], "%lg", &alpha) != 1) {
	    Tcl_AppendResult(interp, "not an angle in radians: ", argv[3], NULL);
	    return TCL_ERROR;
	}
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (LIST_GET(&vgp->poly, i).id == polyid) {
		const size_t n = LIST_GET(&vgp->poly, i).boundary.pn;
		ps = LIST_GET(&vgp->poly, i).boundary.ps;
		p = center(ps, n);
		for (size_t j = 0; j < n; j++) {
		    appendpoint(interp, rotate(p, ps[j], alpha));
		}
		return TCL_OK;
	    }
	}
	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;

    } else if (strcmp(argv[1], "scale") == 0) {
	if (argc < 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id gain\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[3], "%lg", &gain) != 1) {
	    Tcl_AppendResult(interp, "not a number: ", argv[3], NULL);
	    return TCL_ERROR;
	}
	for (size_t i = 0; i < LIST_SIZE(&vgp->poly); i++) {
	    if (LIST_GET(&vgp->poly, i).id == polyid) {
		const size_t n = LIST_GET(&vgp->poly, i).boundary.pn;
		ps = LIST_GET(&vgp->poly, i).boundary.ps;
		p = center(ps, n);
		for (size_t j = 0; j < n; j++) {
		    appendpoint(interp, scale(p, ps[j], gain));
		}
		return TCL_OK;
	    }
	}
	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;

    } else if (strcmp(argv[1], "remove") == 0) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			     " ", argv[1], " id\"", NULL);
	    return TCL_ERROR;
	}
	if (sscanf(argv[2], "%d", &polyid) != 1) {
	    Tcl_AppendResult(interp, "not an integer: ", argv[2], NULL);
	    return TCL_ERROR;
	}

	if (remove_poly(vgp, polyid))
	    return TCL_OK;

	Tcl_AppendResult(interp, " no such polygon: ", argv[2], NULL);
	return TCL_ERROR;
    }

    Tcl_AppendResult(interp, "bad method \"", argv[1],
		     "\" must be one of:",
		     "\n\tbbox, bind, bpath, center, coords, delete, find,",
		     "\n\tinsert, list, path, remove, rotate, scale, triangulate.", NULL);
    return TCL_ERROR;
}

static int
vgpane(ClientData clientData, Tcl_Interp * interp, int argc, const char *argv[])
{
    (void)clientData;
    (void)argc;
    (void)argv;

    const vgpane_t vg = {.interp = interp};
    LIST_APPEND(&vgpaneTable, vg);
    vgpane_t *const vgp = LIST_BACK(&vgpaneTable);
    vgp->index = LIST_SIZE(&vgpaneTable) - 1;
    vgp->valid = true;

    agxbuf buffer = {0};
    agxbprint(&buffer, HANDLE_FORMAT, vgp->index);
    const char *const vbuf = agxbuse(&buffer);

    Tcl_CreateCommand(interp, vbuf, vgpanecmd, NULL, NULL);
    Tcl_AppendResult(interp, vbuf, NULL);
    agxbfree(&buffer);
    return TCL_OK;
}

#ifdef GVDLL
__declspec(dllexport)
#endif
int Tclpathplan_Init(Tcl_Interp *interp);
int Tclpathplan_Init(Tcl_Interp * interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
    }
#endif
    // inter-release Graphviz versions have a number including '~dev.' that does
    // not comply with TCL version number rules, so replace this with 'b'
    char adjusted_version[sizeof(PACKAGE_VERSION)] = PACKAGE_VERSION;
    char *tilde_dev = strstr(adjusted_version, "~dev.");
    if (tilde_dev != NULL) {
	*tilde_dev = 'b';
	memmove(tilde_dev + 1, tilde_dev + strlen("~dev."),
	        strlen(tilde_dev + strlen("~dev.")) + 1);
    }
    if (Tcl_PkgProvide(interp, "Tclpathplan", adjusted_version) != TCL_OK) {
	return TCL_ERROR;
    }

    Tcl_CreateCommand(interp, "vgpane", vgpane, NULL, NULL);

    return TCL_OK;
}

#ifdef GVDLL
__declspec(dllexport)
#endif
int Tclpathplan_SafeInit(Tcl_Interp *interp);
int Tclpathplan_SafeInit(Tcl_Interp * interp)
{
    return Tclpathplan_Init(interp);
}
