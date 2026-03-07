/// @file
/// @brief reads graphs with @ref gvNextInputGraph and renders with
/// @ref gvLayoutJobs, @ref gvRenderJobs

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <graphviz/gvc.h>
#include <stddef.h>

int main(int argc, char **argv) {
  GVC_t *gvc = gvContext();
  gvParseArgs(gvc, argc, argv);

  graph_t *g, *prev = NULL;
  while ((g = gvNextInputGraph(gvc))) {
    if (prev) {
      gvFreeLayout(gvc, prev);
      agclose(prev);
    }
    gvLayoutJobs(gvc, g);
    gvRenderJobs(gvc, g);
    prev = g;
  }
  return gvFreeContext(gvc);
}
