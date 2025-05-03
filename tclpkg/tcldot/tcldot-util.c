/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "../tcl-compat.h"
#include <math.h>
#include <stddef.h>
#include "tcldot.h"
#include <gvc/gvc.h>
#include <util/alloc.h>
#include <util/strcasecmp.h>
#include <util/streq.h>
#include <util/unreachable.h>

size_t Tcldot_string_writer(GVJ_t *job, const char *s, size_t len)
{
    tcldot_context_t *context = job->context;
    Tcl_AppendResult(context->interp, s, NULL);
    return len;
}

size_t Tcldot_channel_writer(GVJ_t *job, const char *s, size_t len)
{
  const Tcl_Size l = len > TCL_SIZE_MAX ? TCL_SIZE_MAX : (Tcl_Size)len;
  const Tcl_Size written = Tcl_Write((Tcl_Channel)job->output_file, s, l);
  if (written < 0) {
    return 0;
  }
  return (size_t)written;
}

/* handles (tcl commands) to obj* */

Agraph_t *cmd2g(const char *cmd) {
    Agraph_t *g = NULL;

    if (sscanf(cmd, "graph%p", &g) != 1 || !g)
        return NULL;
    return g;
}
Agnode_t *cmd2n(const char *cmd) {
    Agnode_t *n = NULL;

    if (sscanf(cmd, "node%p", &n) != 1 || !n)
        return NULL;
    return n;
}
Agedge_t *cmd2e(const char *cmd) {
    Agedge_t *e = NULL;

    if (sscanf(cmd, "edge%p", &e) != 1 || !e)
        return NULL;
    return e;
}


/* obj* to handles (tcl commands) */

char *obj2cmd (void *obj) {
    static char buf[32];

    switch (AGTYPE(obj)) {
        case AGRAPH:    snprintf(buf, sizeof(buf), "graph%p", obj); break;
        case AGNODE:    snprintf(buf, sizeof(buf), "node%p",  obj); break;
        case AGINEDGE: 
        case AGOUTEDGE: snprintf(buf, sizeof(buf), "edge%p",  obj); break;
        default:        UNREACHABLE();
    }
    return buf;
}


void deleteEdge(gctx_t *gctx, Agraph_t * g, Agedge_t *e)
{
    (void)g;

    char *const hndl = obj2cmd(e);
    agdelete(gctx->g, e);  /* delete edge from root graph */
    Tcl_DeleteCommand(gctx->ictx->interp, hndl);
}
static void deleteNodeEdges(gctx_t *gctx, Agraph_t *g, Agnode_t *n)
{
    for (Agedge_t *e = agfstedge(g, n); e != NULL; ) {
	Agedge_t *const e1 = agnxtedge(g, e, n);
	deleteEdge(gctx, g, e);
	e = e1;
    }
}
void deleteNode(gctx_t * gctx, Agraph_t *g, Agnode_t *n)
{
    (void)g;

    deleteNodeEdges(gctx, gctx->g, n); /* delete all edges to/from node in root graph */

    char *const hndl = obj2cmd(n);
    agdelete(gctx->g, n); /* delete node from root graph */
    Tcl_DeleteCommand(gctx->ictx->interp, hndl);
}
static void deleteGraphNodes(gctx_t * gctx, Agraph_t *g)
{
    for (Agnode_t *n = agfstnode(g); n != NULL; ) {
	Agnode_t *const n1 = agnxtnode(g, n);
	deleteNode(gctx, g, n);
	n = n1;
    }
}
void deleteGraph(gctx_t * gctx, Agraph_t *g)
{
    for (Agraph_t *sg = agfstsubg (g); sg; sg = agnxtsubg (sg)) {
	deleteGraph(gctx, sg);
    }
    deleteGraphNodes(gctx, g);

    char *const hndl = obj2cmd(g);
    Tcl_DeleteCommand(gctx->ictx->interp, hndl);
    if (g == agroot(g)) {
	agclose(g);
    } else {
	agdelsubg(agroot(g), g);
    }
}

static void myagxset(void *obj, Agsym_t *a, const char *val) {
    if (streq(a->name, "label") && val[0] == '<') {
        size_t len = strlen(val);
        if (val[len-1] == '>') {
            char *const hs = gv_strdup(val + 1);
                *(hs+len-2) = '\0';
            val = agstrdup_html(agraphof(obj),hs);
            free(hs);
        }
    }
    agxset(obj, a, val);
}

void setgraphattributes(Agraph_t *g, char *argv[], Tcl_Size argc) {
    Agsym_t *a;

    for (Tcl_Size i = 0; i < argc; i++) {
	if (!(a = agfindgraphattr(agroot(g), argv[i])))
	    a = agattr_text(agroot(g), AGRAPH, argv[i], "");
	myagxset(g, a, argv[++i]);
    }
}

void setedgeattributes(Agraph_t *g, Agedge_t *e, char *argv[], Tcl_Size argc) {
    Agsym_t *a;

    for (Tcl_Size i = 0; i < argc; i++) {
	/* silently ignore attempts to modify "key" */
	if (streq(argv[i], "key")) {
	    i++;
	    continue;
	}
	if (e) {
	    if (!(a = agfindedgeattr(g, argv[i])))
		a = agattr_text(agroot(g), AGEDGE, argv[i], "");
	    myagxset(e, a, argv[++i]);
	}
	else {
	    agattr_text(g, AGEDGE, argv[i], argv[i + 1]);
	    i++;
	}
    }
}

void setnodeattributes(Agraph_t *g, Agnode_t *n, char *argv[], Tcl_Size argc) {
    Agsym_t *a;

    for (Tcl_Size i = 0; i < argc; i++) {
	if (n) {
	    if (!(a = agfindnodeattr(g, argv[i])))
		a = agattr_text(agroot(g), AGNODE, argv[i], "");
	    myagxset(n, a, argv[++i]);
	}
	else {
	    agattr_text(g, AGNODE, argv[i], argv[i + 1]);
	    i++;
	}
    }
}

void listGraphAttrs (Tcl_Interp * interp, Agraph_t* g)
{
    Agsym_t *a = NULL;
    while ((a = agnxtattr(g, AGRAPH, a))) {
	Tcl_AppendElement(interp, a->name);
    }
}
void listNodeAttrs (Tcl_Interp * interp, Agraph_t* g)
{
    Agsym_t *a = NULL;
    while ((a = agnxtattr(g, AGNODE, a))) {
	Tcl_AppendElement(interp, a->name);
    }
}
void listEdgeAttrs (Tcl_Interp * interp, Agraph_t* g)
{
    Agsym_t *a = NULL;
    while ((a = agnxtattr(g, AGEDGE, a))) {
	Tcl_AppendElement(interp, a->name);
    }
}

void tcldot_layout(GVC_t *gvc, Agraph_t * g, const char *engine)
{
    gvFreeLayout(gvc, g);               /* in case previously drawn */

/* support old behaviors if engine isn't specified*/
    if (!engine || *engine == '\0') {
	if (agisdirected(g))
	    engine = "dot";
	else
	    engine = "neato";
    }
    else {
	if (strcasecmp(engine, "nop") == 0) {
	    Nop = 2;
	    PSinputscale = POINTS_PER_INCH;
	    engine = "neato";
	}
    }
    gvLayout(gvc, g, engine);
}

char **tcldot_argv_dup(Tcl_Size argc, const char *argv[]) {
  assert(argc > 0);
  char **argv_ret = gv_calloc((size_t)argc, sizeof(char *));
  for (Tcl_Size i = 0; i < argc; ++i) {
    argv_ret[i] = gv_strdup(argv[i]);
  }
  return argv_ret;
}

void tcldot_argv_free(Tcl_Size argc, char *argv[]) {
  for (Tcl_Size i = 0; i < argc; ++i) {
    free(argv[i]);
  }
  free(argv);
}
