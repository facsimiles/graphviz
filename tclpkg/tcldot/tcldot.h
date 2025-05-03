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
#include <tcl.h>
#ifdef EXTERN
// tcl.h defines `EXTERN` which interferes with the `EXTERN` in globals.h
#undef EXTERN
#endif
#include <common/render.h>
#include <gvc/gvc.h>
#include <gvc/gvio.h>
#include "../../plugin/core/tcl_context.h"

/*
 * ictx - one per tcl interpreter, may support multiple graph namespaces
 */
typedef struct {
    Agdisc_t mydisc;    /* must be first to allow casting mydisc to ictx */
    Agiodisc_t myioDisc;
    uint64_t ctr;  /* odd number counter for anon objects over all g's in interp */
    Tcl_Interp *interp;
    GVC_t *gvc;
} ictx_t;

/*
 * gctx - one for each graph in a tcl interp
 */
typedef struct {
    Agraph_t *g;        /* the graph */
    ictx_t *ictx;
} gctx_t;

#ifdef HAVE_LIBGD
extern void *GDHandleTable;
extern int Gdtclft_Init(Tcl_Interp *);
#endif

extern int graphcmd(ClientData clientData, Tcl_Interp * interp, int argc, const char *argv[]);
extern int nodecmd(ClientData clientData, Tcl_Interp * interp, int argc, const char *argv[]);
extern int edgecmd(ClientData clientData, Tcl_Interp * interp, int argc, const char *argv[]);

extern int myiodisc_afread(void* channel, char *ubuf, int n);
extern int myiodisc_memiofread(void *chan, char *buf, int bufsize);
extern Agiddisc_t myiddisc;
extern Agraph_t *cmd2g(const char *cmd);
extern Agnode_t *cmd2n(const char *cmd);
extern Agedge_t *cmd2e(const char *cmd);
extern char *obj2cmd(void *obj);
extern void deleteEdge(gctx_t *gctx, Agraph_t * g, Agedge_t * e);
extern void deleteNode(gctx_t *gctx, Agraph_t * g, Agnode_t * n);
extern void deleteGraph(gctx_t *gctx, Agraph_t * g);
extern void listGraphAttrs (Tcl_Interp * interp, Agraph_t* g);
extern void listNodeAttrs (Tcl_Interp * interp, Agraph_t* g);
extern void listEdgeAttrs (Tcl_Interp * interp, Agraph_t* g);

void setgraphattributes(Agraph_t *g, char *argv[], Tcl_Size argc);
void setedgeattributes(Agraph_t *g, Agedge_t *e, char *argv[], Tcl_Size argc);
void setnodeattributes(Agraph_t *g, Agnode_t *n, char *argv[], Tcl_Size argc);

extern size_t Tcldot_string_writer(GVJ_t *job, const char *s, size_t len);
extern size_t Tcldot_channel_writer(GVJ_t *job, const char *s, size_t len);

extern void tcldot_layout(GVC_t *gvc, Agraph_t * g, const char *engine);

/// duplicate the strings pointed to by `argv` as non-const strings
char **tcldot_argv_dup(Tcl_Size argc, const char *argv[]);
/// free the strings pointed to by `argv`
void tcldot_argv_free(Tcl_Size argc, char *argv[]);
