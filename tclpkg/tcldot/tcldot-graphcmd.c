/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "tcldot.h"
#include <stdbool.h>
#include <string.h>
#include <util/streq.h>

static int graphcmd_internal(ClientData clientData, Tcl_Interp *interp,
                             int argc, char *argv[]) {
  Agraph_t *g, *sg;
  Agnode_t *n, *tail, *head;
  Agedge_t *e;
  gctx_t *gctx = (gctx_t *)clientData;
  ictx_t *ictx = gctx->ictx;
  Agsym_t *a;
  char buf[12], **argv2;
  int j, argc2;
  GVC_t *gvc = ictx->gvc;

  if (argc < 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " option ?arg arg ...?\"", NULL);
    return TCL_ERROR;
  }
  g = cmd2g(argv[0]);
  if (!g) {
    Tcl_AppendResult(interp, "graph \"", argv[0], "\" not found", NULL);
    return TCL_ERROR;
  }

  if (streq("addedge", argv[1])) {
    if ((argc < 4) || (argc % 2)) {
      Tcl_AppendResult(
          interp, "wrong # args: should be \"", argv[0],
          " addedge tail head ?attributename attributevalue? ?...?\"", NULL);
      return TCL_ERROR;
    }
    tail = cmd2n(argv[2]);
    if (!tail) {
      if (!(tail = agfindnode(g, argv[2]))) {
        Tcl_AppendResult(interp, "tail node \"", argv[2], "\" not found.",
                         NULL);
        return TCL_ERROR;
      }
    }
    if (agroot(g) != agroot(agraphof(tail))) {
      Tcl_AppendResult(interp, "tail node ", argv[2], " is not in the graph.",
                       NULL);
      return TCL_ERROR;
    }
    head = cmd2n(argv[3]);
    if (!head) {
      if (!(head = agfindnode(g, argv[3]))) {
        Tcl_AppendResult(interp, "head node \"", argv[3], "\" not found.",
                         NULL);
        return TCL_ERROR;
      }
    }
    if (agroot(g) != agroot(agraphof(head))) {
      Tcl_AppendResult(interp, "head node ", argv[3], " is not in the graph.",
                       NULL);
      return TCL_ERROR;
    }
    e = agedge(g, tail, head, NULL, 1);
    Tcl_AppendResult(interp, obj2cmd(e), NULL);
    setedgeattributes(agroot(g), e, &argv[4], argc - 4);
    return TCL_OK;

  } else if (streq("addnode", argv[1])) {
    int i;
    if (argc % 2) {
      /* if odd number of args then argv[2] is name */
      n = agnode(g, argv[2], 1);
      i = 3;
    } else {
      n = agnode(g, NULL, 1); /* anon node */
      i = 2;
    }
    Tcl_AppendResult(interp, obj2cmd(n), NULL);
    setnodeattributes(agroot(g), n, &argv[i], argc - i);
    return TCL_OK;

  } else if (streq("addsubgraph", argv[1])) {
    int i;
    if (argc < 2) {
      Tcl_AppendResult(
          interp, "wrong # args: should be \"", argv[0],
          "\" addsubgraph ?name? ?attributename attributevalue? ?...?", NULL);
    }
    if (argc % 2) {
      /* if odd number of args then argv[2] is name */
      sg = agsubg(g, argv[2], 1);
      Tcl_AppendResult(interp, obj2cmd(sg), NULL);
      i = 3;
    } else {
      sg = agsubg(g, NULL, 1); /* anon subgraph */
      i = 2;
    }
    setgraphattributes(sg, &argv[i], argc - i);
    return TCL_OK;

  } else if (streq("countnodes", argv[1])) {
    snprintf(buf, sizeof(buf), "%d", agnnodes(g));
    Tcl_AppendResult(interp, buf, NULL);
    return TCL_OK;

  } else if (streq("countedges", argv[1])) {
    snprintf(buf, sizeof(buf), "%d", agnedges(g));
    Tcl_AppendResult(interp, buf, NULL);
    return TCL_OK;

  } else if (streq("delete", argv[1])) {
    deleteGraph(gctx, g);
    return TCL_OK;

  } else if (streq("findedge", argv[1])) {
    if (argc < 4) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                       " findedge tailnodename headnodename\"", NULL);
      return TCL_ERROR;
    }
    if (!(tail = agfindnode(g, argv[2]))) {
      Tcl_AppendResult(interp, "tail node \"", argv[2], "\" not found.", NULL);
      return TCL_ERROR;
    }
    if (!(head = agfindnode(g, argv[3]))) {
      Tcl_AppendResult(interp, "head node \"", argv[3], "\" not found.", NULL);
      return TCL_ERROR;
    }
    if (!(e = agfindedge(g, tail, head))) {
      Tcl_AppendResult(interp, "edge \"", argv[2], " - ", argv[3],
                       "\" not found.", NULL);
      return TCL_ERROR;
    }
    Tcl_AppendElement(interp, obj2cmd(e));
    return TCL_OK;

  } else if (streq("findnode", argv[1])) {
    if (argc < 3) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                       " findnode nodename\"", NULL);
      return TCL_ERROR;
    }
    if (!(n = agfindnode(g, argv[2]))) {
      Tcl_AppendResult(interp, "node not found.", NULL);
      return TCL_ERROR;
    }
    Tcl_AppendResult(interp, obj2cmd(n), NULL);
    return TCL_OK;

  } else if (streq("layoutedges", argv[1])) {
    g = agroot(g);
    if (!aggetrec(g, "Agraphinfo_t", 0))
      tcldot_layout(gvc, g, (argc > 2) ? argv[2] : NULL);
    return TCL_OK;

  } else if (streq("layoutnodes", argv[1])) {
    g = agroot(g);
    if (!aggetrec(g, "Agraphinfo_t", 0))
      tcldot_layout(gvc, g, (argc > 2) ? argv[2] : NULL);
    return TCL_OK;

  } else if (streq("listattributes", argv[1])) {
    listGraphAttrs(interp, g);
    return TCL_OK;

  } else if (streq("listedgeattributes", argv[1])) {
    listEdgeAttrs(interp, g);
    return TCL_OK;

  } else if (streq("listnodeattributes", argv[1])) {
    listNodeAttrs(interp, g);
    return TCL_OK;

  } else if (streq("listedges", argv[1])) {
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
      for (e = agfstout(g, n); e; e = agnxtout(g, e)) {
        Tcl_AppendElement(interp, obj2cmd(e));
      }
    }
    return TCL_OK;

  } else if (streq("listnodes", argv[1])) {
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
      Tcl_AppendElement(interp, obj2cmd(n));
    }
    return TCL_OK;

  } else if (streq("listnodesrev", argv[1])) {
    for (n = aglstnode(g); n; n = agprvnode(g, n)) {
      Tcl_AppendElement(interp, obj2cmd(n));
    }
    return TCL_OK;

  } else if (streq("listsubgraphs", argv[1])) {
    for (sg = agfstsubg(g); sg; sg = agnxtsubg(sg)) {
      Tcl_AppendElement(interp, obj2cmd(sg));
    }
    return TCL_OK;

  } else if (streq("queryattributes", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindgraphattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("queryattributevalues", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindgraphattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, argv2[j]);
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("queryedgeattributes", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindedgeattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("queryedgeattributevalues", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindedgeattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, argv2[j]);
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("querynodeattributes", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindnodeattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("querynodeattributevalues", argv[1])) {
    for (int i = 2; i < argc; i++) {
      if (Tcl_SplitList(interp, argv[i], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      for (j = 0; j < argc2; j++) {
        if ((a = agfindnodeattr(g, argv2[j]))) {
          Tcl_AppendElement(interp, argv2[j]);
          Tcl_AppendElement(interp, agxget(g, a));
        } else {
          Tcl_AppendResult(interp, " No attribute named \"", argv2[j], "\"",
                           NULL);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("render", argv[1])) {
    const char *canvas;

    if (argc < 3) {
      canvas = "$c";
    } else {
      canvas = argv[2];
    }

    gvc->write_fn = Tcldot_string_writer;
    tcldot_context_t context = {.canvas = canvas, .interp = interp};

    /* make sure that layout is done */
    g = agroot(g);
    if (!aggetrec(g, "Agraphinfo_t", 0) || argc > 3)
      tcldot_layout(gvc, g, (argc > 3) ? argv[3] : NULL);

    /* render graph TK canvas commands */
    gvc->common.viewNum = 0;
    if (gvRenderContext(gvc, g, "tk", &context) != 0) {
      return TCL_ERROR;
    }
    fflush(stdout);
    return TCL_OK;

  } else if (streq("setattributes", argv[1])) {
    if (argc == 3) {
      if (Tcl_SplitList(interp, argv[2], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      if ((argc2 == 0) || (argc2 % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        Tcl_Free((char *)argv2);
        return TCL_ERROR;
      }
      setgraphattributes(g, argv2, argc2);
      Tcl_Free((char *)argv2);
    }
    if (argc == 4 && streq(argv[2], "viewport")) {
      /* special case to allow viewport to be set without resetting layout */
      setgraphattributes(g, &argv[2], argc - 2);
    } else {
      if ((argc < 4) || (argc % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        return TCL_ERROR;
      }
      setgraphattributes(g, &argv[2], argc - 2);
    }
    return TCL_OK;

  } else if (streq("setedgeattributes", argv[1])) {
    if (argc == 3) {
      if (Tcl_SplitList(interp, argv[2], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      if ((argc2 == 0) || (argc2 % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setedgeattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        Tcl_Free((char *)argv2);
        return TCL_ERROR;
      }
      setedgeattributes(g, NULL, argv2, argc2);
      Tcl_Free((char *)argv2);
    } else {
      if ((argc < 4) || (argc % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setedgeattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
      }
      setedgeattributes(g, NULL, &argv[2], argc - 2);
    }
    return TCL_OK;

  } else if (streq("setnodeattributes", argv[1])) {
    if (argc == 3) {
      if (Tcl_SplitList(interp, argv[2], &argc2, (const char ***)&argv2) !=
          TCL_OK)
        return TCL_ERROR;
      if ((argc2 == 0) || (argc2 % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setnodeattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        Tcl_Free((char *)argv2);
        return TCL_ERROR;
      }
      setnodeattributes(g, NULL, argv2, argc2);
      Tcl_Free((char *)argv2);
    } else {
      if ((argc < 4) || (argc % 2)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setnodeattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
      }
      setnodeattributes(g, NULL, &argv[2], argc - 2);
    }
    return TCL_OK;

  } else if (streq("showname", argv[1])) {
    Tcl_SetResult(interp, agnameof(g), TCL_STATIC);
    return TCL_OK;
  } else if (streq("write", argv[1])) {
    g = agroot(g);
    if (argc < 3) {
      Tcl_AppendResult(
          interp, "wrong # args: should be \"", argv[0],
          " write fileHandle ?language ?DOT|NEATO|TWOPI|FDP|CIRCO|NOP??\"",
          NULL);
      return TCL_ERROR;
    }

    gvc->write_fn = Tcldot_channel_writer;

    Tcl_Channel chan;
    {
      int mode;

      chan = Tcl_GetChannel(interp, argv[2], &mode);

      if (!chan) {
        Tcl_AppendResult(interp, "channel not open: \"", argv[2], NULL);
        return TCL_ERROR;
      }
      if (!(mode & TCL_WRITABLE)) {
        Tcl_AppendResult(interp, "channel not writable: \"", argv[2], NULL);
        return TCL_ERROR;
      }
    }

    /* make sure that layout is done  - unless canonical output */
    if (!aggetrec(g, "Agraphinfo_t", 0) || argc > 4)
      tcldot_layout(gvc, g, (argc > 4) ? argv[4] : NULL);

    gvc->common.viewNum = 0;
    if (gvRender(gvc, g, argc < 4 ? "dot" : argv[3], (FILE *)chan) != 0) {
      return TCL_ERROR;
    }
    return TCL_OK;

  } else {
    Tcl_AppendResult(
        interp, "bad option \"", argv[1], "\": must be one of:",
        "\n\taddedge, addnode, addsubgraph, countedges, countnodes,",
        "\n\tlayout, listattributes, listedgeattributes, listnodeattributes,",
        "\n\tlistedges, listnodes, listsubgraphs, render, rendergd,",
        "\n\tqueryattributes, queryedgeattributes, querynodeattributes,",
        "\n\tqueryattributevalues, queryedgeattributevalues, "
        "querynodeattributevalues,",
        "\n\tsetattributes, setedgeattributes, setnodeattributes,",
        "\n\tshowname, write.", NULL);
    return TCL_ERROR;
  }
} /* graphcmd */

int graphcmd(ClientData clientData, Tcl_Interp *interp, int argc,
             const char *argv[]) {
  char **argv_copy = tcldot_argv_dup(argc, argv);
  int rc = graphcmd_internal(clientData, interp, argc, argv_copy);
  tcldot_argv_free(argc, argv_copy);
  return rc;
}
