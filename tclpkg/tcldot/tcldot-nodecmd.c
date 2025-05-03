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
#include "tcldot.h"
#include <stdlib.h>
#include <string.h>
#include <util/streq.h>

static int nodecmd_internal(ClientData clientData, Tcl_Interp *interp, int argc,
                            char *argv[]) {
  const char **argv2;
  int i;
  Agraph_t *g;
  Agnode_t *n, *head;
  Agedge_t *e;
  Agsym_t *a;
  gctx_t *gctx = (gctx_t *)clientData;

  if (argc < 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " option ?arg arg ...?\"", NULL);
    return TCL_ERROR;
  }
  n = cmd2n(argv[0]);
  if (!n) {
    Tcl_AppendResult(interp, "node \"", argv[0], "\" not found", NULL);
    return TCL_ERROR;
  }
  g = agraphof(n);

  if (streq("addedge", argv[1])) {
    if (argc < 3 || argc % 2 == 0) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                       " addedge head ?attributename attributevalue? ?...?\"",
                       NULL);
      return TCL_ERROR;
    }
    head = cmd2n(argv[2]);
    if (!head) {
      if (!(head = agfindnode(g, argv[2]))) {
        Tcl_AppendResult(interp, "head node \"", argv[2], "\" not found.",
                         NULL);
        return TCL_ERROR;
      }
    }
    if (agroot(g) != agroot(agraphof(head))) {
      Tcl_AppendResult(interp, "nodes ", argv[0], " and ", argv[2],
                       " are not in the same graph.", NULL);
      return TCL_ERROR;
    }
    e = agedge(g, n, head, NULL, 1);
    Tcl_AppendResult(interp, obj2cmd(e), NULL);
    setedgeattributes(agroot(g), e, &argv[3], (Tcl_Size)argc - 3);
    return TCL_OK;

  } else if (streq("delete", argv[1])) {
    deleteNode(gctx, g, n);
    return TCL_OK;

  } else if (streq("findedge", argv[1])) {
    if (argc < 3) {
      Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                       " findedge headnodename\"", NULL);
      return TCL_ERROR;
    }
    if (!(head = agfindnode(g, argv[2]))) {
      Tcl_AppendResult(interp, "head node \"", argv[2], "\" not found.", NULL);
      return TCL_ERROR;
    }
    if (!(e = agfindedge(g, n, head))) {
      Tcl_AppendResult(interp, "edge \"", argv[0], " - ", obj2cmd(head),
                       "\" not found.", NULL);
      return TCL_ERROR;
    }
    Tcl_AppendElement(interp, obj2cmd(head));
    return TCL_OK;

  } else if (streq("listattributes", argv[1])) {
    listNodeAttrs(interp, g);
    return TCL_OK;

  } else if (streq("listedges", argv[1])) {
    for (e = agfstedge(g, n); e; e = agnxtedge(g, e, n)) {
      Tcl_AppendElement(interp, obj2cmd(e));
    }
    return TCL_OK;

  } else if (streq("listinedges", argv[1])) {
    for (e = agfstin(g, n); e; e = agnxtin(g, e)) {
      Tcl_AppendElement(interp, obj2cmd(e));
    }
    return TCL_OK;

  } else if (streq("listoutedges", argv[1])) {
    for (e = agfstout(g, n); e; e = agnxtout(g, e)) {
      Tcl_AppendElement(interp, obj2cmd(e));
    }
    return TCL_OK;

  } else if (streq("queryattributes", argv[1])) {
    for (i = 2; i < argc; i++) {
      Tcl_Size argc2;
      if (Tcl_SplitList(interp, argv[i], &argc2, &argv2) != TCL_OK)
        return TCL_ERROR;
      for (Tcl_Size j = 0; j < argc2; j++) {
        char *arg = strdup(argv2[j]);
        if ((a = agfindnodeattr(g, arg))) {
          Tcl_AppendElement(interp, agxget(n, a));
        } else {
          Tcl_AppendResult(interp, "no attribute named \"", arg, "\"", NULL);
          free(arg);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
        free(arg);
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("queryattributevalues", argv[1])) {
    for (i = 2; i < argc; i++) {
      Tcl_Size argc2;
      if (Tcl_SplitList(interp, argv[i], &argc2, &argv2) != TCL_OK)
        return TCL_ERROR;
      for (Tcl_Size j = 0; j < argc2; j++) {
        char *arg = strdup(argv2[j]);
        if ((a = agfindnodeattr(g, arg))) {
          Tcl_AppendElement(interp, arg);
          Tcl_AppendElement(interp, agxget(n, a));
        } else {
          Tcl_AppendResult(interp, "no attribute named \"", arg, "\"", NULL);
          free(arg);
          Tcl_Free((char *)argv2);
          return TCL_ERROR;
        }
        free(arg);
      }
      Tcl_Free((char *)argv2);
    }
    return TCL_OK;

  } else if (streq("setattributes", argv[1])) {
    g = agroot(g);
    if (argc == 3) {
      Tcl_Size argc2;
      if (Tcl_SplitList(interp, argv[2], &argc2, &argv2) != TCL_OK)
        return TCL_ERROR;
      if (argc2 == 0 || argc2 % 2 != 0) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        Tcl_Free((char *)argv2);
        return TCL_ERROR;
      }
      char **argv2_copy = tcldot_argv_dup(argc2, argv2);
      setnodeattributes(g, n, argv2_copy, argc2);
      tcldot_argv_free(argc2, argv2_copy);
      Tcl_Free((char *)argv2);
    } else {
      if (argc < 4 || argc % 2 != 0) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         "\" setattributes attributename attributevalue "
                         "?attributename attributevalue? ?...?",
                         NULL);
        return TCL_ERROR;
      }
      setnodeattributes(g, n, &argv[2], (Tcl_Size)argc - 2);
    }
    return TCL_OK;

  } else if (streq("showname", argv[1])) {
    Tcl_SetResult(interp, agnameof(n), TCL_STATIC);
    return TCL_OK;

  } else {
    Tcl_AppendResult(interp, "bad option \"", argv[1], "\": must be one of:",
                     "\n\taddedge, listattributes, listedges, listinedges,",
                     "\n\tlistoutedges, queryattributes, queryattributevalues,",
                     "\n\tsetattributes, showname.", NULL);
    return TCL_ERROR;
  }
}

int nodecmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char *argv[]) {
  char **argv_copy = tcldot_argv_dup((Tcl_Size)argc, argv);
  int rc = nodecmd_internal(clientData, interp, argc, argv_copy);
  tcldot_argv_free((Tcl_Size)argc, argv_copy);
  return rc;
}
