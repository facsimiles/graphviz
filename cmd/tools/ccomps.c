/**
 * @file
 * @brief connected components filter for graphs
 */

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
 * Written by Stephen North
 * Updated by Emden Gansner
 */

#include <assert.h>
#include <cgraph/cgraph.h>
#include <cgraph/ingraphs.h>
#include <common/render.h>
#include <common/utils.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/gv_ctype.h>
#include <util/list.h>
#include <util/prisize_t.h>
#include <util/strview.h>
#include <util/unreachable.h>

typedef struct {
  Agrec_t h;
  bool cc_subg; ///< true iff subgraph corresponds to a component
} graphinfo_t;

typedef struct {
  Agrec_t h;
  char mark;
  Agobj_t *ptr;
} nodeinfo_t;

#define GD_cc_subg(g) (((graphinfo_t *)(g->base.data))->cc_subg)
#define Node_mark(n) (((nodeinfo_t *)(n->base.data))->mark)
#define ND_ptr(n) (((nodeinfo_t *)(n->base.data))->ptr)
#define ND_dn(n) ((Agnode_t *)ND_ptr(n))
#define Node_clust(n) ((Agraph_t *)ND_ptr(n))

static char **Inputs;
static bool verbose = false;
static enum {
  INTERNAL, ///< all components need to be generated before output
  EXTERNAL,
  SILENT,
  EXTRACT,
} printMode = INTERNAL;
static bool useClusters = false;
static bool doEdges = true; ///< induce edges
static bool doAll = true;   ///< induce subgraphs
static char *suffix = 0;
static char *outfile = 0;
static strview_t rootpath;
static int sufcnt = 0;
static bool sorted = false;
static int sortIndex = 0;
static int sortFinal;
static int x_index = -1;
static int x_final = -1; // require 0 <= x_index <= x_final or x_final= -1
static enum { BY_INDEX = 1, BY_SIZE = 2 } x_mode;
static char *x_node;

static char *useString =
    "Usage: ccomps [-svenCx?] [-X[#%]s[-f]] [-o<out template>] <files>\n\
  -s - silent\n\
  -x - external\n\
  -X - extract component\n\
  -C - use clusters\n\
  -e - do not induce edges\n\
  -n - do not induce subgraphs\n\
  -v - verbose\n\
  -o - output file template\n\
  -z - sort by size, largest first\n\
  -? - print usage\n\
If no files are specified, stdin is used\n";

static void usage(int v) {
  printf("%s", useString);
  graphviz_exit(v);
}

static void split(void) {
  char *sfx = strrchr(outfile, '.');
  if (sfx) {
    suffix = sfx + 1;
    size_t size = (size_t)(sfx - outfile);
    rootpath = (strview_t){.data = outfile, .size = size};
  } else {
    rootpath = strview(outfile, '\0');
  }
}

static void init(int argc, char *argv[]) {
  int c;
  char *endp;

  opterr = 0;
  while ((c = getopt(argc, argv, ":zo:xCX:nesv?")) != -1) {
    switch (c) {
    case 'o':
      outfile = optarg;
      split();
      break;
    case 'C':
      useClusters = true;
      break;
    case 'e':
      doEdges = false;
      break;
    case 'n':
      doAll = false;
      break;
    case 'x':
      printMode = EXTERNAL;
      break;
    case 's':
      printMode = SILENT;
      break;
    case 'X':
      if (*optarg == '#' || *optarg == '%') {
        char *p = optarg + 1;
        if (*optarg == '#')
          x_mode = BY_INDEX;
        else
          x_mode = BY_SIZE;
        if (gv_isdigit(*p)) {
          x_index = (int)strtol(p, &endp, 10);
          printMode = EXTRACT;
          if (*endp == '-') {
            p = endp + 1;
            if (gv_isdigit(*p)) {
              x_final = atoi(p);
              if (x_final < x_index) {
                printMode = INTERNAL;
                fprintf(stderr,
                        "ccomps: final index %d < start index %d in -X%s flag "
                        "- ignored\n",
                        x_final, x_index, optarg);
              }
            } else if (*p) {
              printMode = INTERNAL;
              fprintf(stderr,
                      "ccomps: number expected in -X%s flag - ignored\n",
                      optarg);
            }
          } else
            x_final = x_index;
        } else
          fprintf(stderr, "ccomps: number expected in -X%s flag - ignored\n",
                  optarg);
      } else {
        x_node = optarg;
        printMode = EXTRACT;
      }
      break;
    case 'v':
      verbose = true;
      break;
    case 'z':
      sorted = true;
      break;
    case ':':
      fprintf(stderr, "ccomps: option -%c missing argument - ignored\n",
              optopt);
      break;
    case '?':
      if (optopt == '\0' || optopt == '?')
        usage(0);
      else {
        fprintf(stderr, "ccomps: option -%c unrecognized\n", optopt);
        usage(1);
      }
      break;
    default:
      UNREACHABLE();
    }
  }
  argv += optind;
  argc -= optind;

  if (sorted) {
    if (printMode == EXTRACT && x_index >= 0) {
      printMode = INTERNAL;
      sortIndex = x_index;
      sortFinal = x_final;
    } else if (printMode == EXTERNAL) {
      sortIndex = -1;
      printMode = INTERNAL;
    } else
      sorted = false; // not relevant; turn off
  }
  if (argc > 0)
    Inputs = argv;
}

DEFINE_LIST(node_stack, Agnode_t *)
static node_stack_t Stk;

static void push(Agnode_t *np) {
  Node_mark(np) = -1;
  node_stack_push_back(&Stk, np);
}

static Agnode_t *pop(void) {
  if (node_stack_is_empty(&Stk)) {
    return NULL;
  }
  return node_stack_pop_back(&Stk);
}

static int dfs(Agraph_t *g, Agnode_t *n, Agraph_t *out) {
  Agnode_t *other;
  int cnt = 0;

  push(n);
  while ((n = pop())) {
    Node_mark(n) = 1;
    cnt++;
    agsubnode(out, n, 1);
    for (Agedge_t *e = agfstedge(g, n); e; e = agnxtedge(g, e, n)) {
      if ((other = agtail(e)) == n)
        other = aghead(e);
      if (Node_mark(other) == 0)
        push(other);
    }
  }
  return cnt;
}

static char *getName(void) {
  agxbuf name = {0};

  if (sufcnt == 0)
    agxbput(&name, outfile);
  else {
    if (suffix)
      agxbprint(&name, "%.*s_%d.%s", (int)rootpath.size, rootpath.data, sufcnt,
                suffix);
    else
      agxbprint(&name, "%.*s_%d", (int)rootpath.size, rootpath.data, sufcnt);
  }
  sufcnt++;
  return agxbdisown(&name);
}

static void gwrite(Agraph_t *g) {
  if (!outfile) {
    agwrite(g, stdout);
    fflush(stdout);
  } else {
    char *const name = getName();
    FILE *const outf = fopen(name, "w");
    if (!outf) {
      fprintf(stderr, "Could not open %s for writing\n", name);
      perror("ccomps");
      free(name);
      graphviz_exit(EXIT_FAILURE);
    }
    free(name);
    agwrite(g, outf);
    fclose(outf);
  }
}

/* If any nodes of subg are in g, create a subgraph of g
 * and fill it with all nodes of subg in g and their induced
 * edges in subg. Copy the attributes of subg to g. Return the subgraph.
 * If not, return null.
 */
static Agraph_t *projectG(Agraph_t *subg, Agraph_t *g, int inCluster) {
  Agraph_t *proj = 0;
  Agnode_t *m;

  for (Agnode_t *n = agfstnode(subg); n; n = agnxtnode(subg, n)) {
    if ((m = agfindnode(g, agnameof(n)))) {
      if (proj == 0) {
        proj = agsubg(g, agnameof(subg), 1);
      }
      agsubnode(proj, m, 1);
    }
  }
  if (!proj && inCluster) {
    proj = agsubg(g, agnameof(subg), 1);
  }
  if (proj) {
    if (doEdges)
      (void)graphviz_node_induce(proj, subg);
    agcopyattr(subg, proj);
  }

  return proj;
}

/* Project subgraphs of root graph on subgraph.
 * If non-empty, add to subgraph.
 */
static void subgInduce(Agraph_t *root, Agraph_t *g, int inCluster) {
  Agraph_t *proj;

  for (Agraph_t *subg = agfstsubg(root); subg; subg = agnxtsubg(subg)) {
    if (GD_cc_subg(subg))
      continue;
    if ((proj = projectG(subg, g, inCluster))) {
      const int in_cluster = inCluster || (useClusters && is_a_cluster(subg));
      subgInduce(subg, proj, in_cluster);
    }
  }
}

static void subGInduce(Agraph_t *g, Agraph_t *out) { subgInduce(g, out, 0); }

#define PFX1 "%s_cc"
#define PFX2 "%s_cc_%" PRISIZE_T

/* Construct nodes in derived graph corresponding top-level clusters.
 * Since a cluster might be wrapped in a subgraph, we need to traverse
 * down into the tree of subgraphs
 */
static void deriveClusters(Agraph_t *dg, Agraph_t *g) {
  for (Agraph_t *subg = agfstsubg(g); subg; subg = agnxtsubg(subg)) {
    if (is_a_cluster(subg)) {
      Agnode_t *const dn = agnode(dg, agnameof(subg), 1);
      agbindrec(dn, "nodeinfo", sizeof(nodeinfo_t), true);
      ND_ptr(dn) = (Agobj_t *)subg;
      for (Agnode_t *n = agfstnode(subg); n; n = agnxtnode(subg, n)) {
        if (ND_ptr(n)) {
          fprintf(stderr,
                  "Error: node \"%s\" belongs to two non-nested clusters "
                  "\"%s\" and \"%s\"\n",
                  agnameof(n), agnameof(subg), agnameof(ND_dn(n)));
        }
        ND_ptr(n) = (Agobj_t *)dn;
      }
    } else {
      deriveClusters(dg, subg);
    }
  }
}

/* Create derived graph dg of g where nodes correspond to top-level nodes
 * or clusters, and there is an edge in dg if there is an edge in g
 * between any nodes in the respective clusters.
 */
static Agraph_t *deriveGraph(Agraph_t *g) {
  Agraph_t *dg = agopen("dg", Agstrictundirected, NULL);

  deriveClusters(dg, g);

  for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
    if (ND_dn(n))
      continue;
    Agnode_t *const dn = agnode(dg, agnameof(n), 1);
    agbindrec(dn, "nodeinfo", sizeof(nodeinfo_t), true);
    ND_ptr(dn) = (Agobj_t *)n;
    ND_ptr(n) = (Agobj_t *)dn;
  }

  for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
    Agnode_t *tl = ND_dn(n);
    for (Agedge_t *e = agfstout(g, n); e; e = agnxtout(g, e)) {
      Agnode_t *const hd = ND_dn(aghead(e));
      if (hd == tl)
        continue;
      if (hd > tl)
        agedge(dg, tl, hd, 0, 1);
      else
        agedge(dg, hd, tl, 0, 1);
    }
  }

  return dg;
}

/// add all nodes in cluster nodes of `dg` to `g`
static void unionNodes(Agraph_t *dg, Agraph_t *g) {
  for (Agnode_t *dn = agfstnode(dg); dn; dn = agnxtnode(dg, dn)) {
    if (AGTYPE(ND_ptr(dn)) == AGNODE) {
      agsubnode(g, ND_dn(dn), 1);
    } else {
      Agraph_t *const clust = Node_clust(dn);
      for (Agnode_t *n = agfstnode(clust); n; n = agnxtnode(clust, n))
        agsubnode(g, n, 1);
    }
  }
}

static int cmp(const void *x, const void *y) {
// Suppress Clang/GCC -Wcast-qual warning. Casting away const here is acceptable
// as the later usage is const. We need the cast because `agnnodes` uses
// non-const pointers.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
  Agraph_t **p0 = (Agraph_t **)x;
  Agraph_t **p1 = (Agraph_t **)y;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
  const int n0 = agnnodes(*p0);
  const int n1 = agnnodes(*p1);
  if (n0 < n1) {
    return 1;
  }
  if (n0 > n1) {
    return -1;
  }
  return 0;
}

static void printSorted(Agraph_t *root, size_t c_cnt) {
  Agraph_t **ccs = gv_calloc(c_cnt, sizeof(Agraph_t *));
  size_t i = 0;

  for (Agraph_t *subg = agfstsubg(root); subg; subg = agnxtsubg(subg)) {
    if (GD_cc_subg(subg))
      ccs[i++] = subg;
  }
  /* sort by component size, largest first */
  qsort(ccs, c_cnt, sizeof(Agraph_t *), cmp);

  if (sortIndex >= 0) {
    if (x_mode == BY_INDEX) {
      if ((size_t)sortIndex >= c_cnt) {
        fprintf(stderr,
                "ccomps: component %d not found in graph %s - ignored\n",
                sortIndex, agnameof(root));
        free(ccs);
        return;
      }
      size_t endi;
      if (sortFinal >= sortIndex) {
        endi = (size_t)sortFinal;
        if (endi >= c_cnt) {
          assert(c_cnt > 0);
          endi = c_cnt - 1;
        }
      } else
        endi = c_cnt - 1;
      for (i = (size_t)sortIndex; i <= endi; i++) {
        Agraph_t *const subg = ccs[i];
        if (doAll)
          subGInduce(root, subg);
        gwrite(subg);
      }
    } else if (x_mode == BY_SIZE) {
      if (sortFinal == -1)
        sortFinal = agnnodes(ccs[0]);
      for (i = 0; i < c_cnt; i++) {
        Agraph_t *const subg = ccs[i];
        int sz = agnnodes(subg);
        if (sz > sortFinal)
          continue;
        if (sz < sortIndex)
          break;
        if (doAll)
          subGInduce(root, subg);
        gwrite(subg);
      }
    }
  } else
    for (i = 0; i < c_cnt; i++) {
      Agraph_t *const subg = ccs[i];
      if (doAll)
        subGInduce(root, subg);
      gwrite(subg);
    }
  free(ccs);
}

/// return 0 if graph is connected
static int processClusters(Agraph_t *g, char *graphName) {
  Agraph_t *out;
  Agraph_t *dout;
  bool extracted = false;

  Agraph_t *const dg = deriveGraph(g);

  if (x_node) {
    Agnode_t *const n = agfindnode(g, x_node);
    if (!n) {
      fprintf(stderr, "ccomps: node %s not found in graph %s\n", x_node,
              agnameof(g));
      return 1;
    }
    {
      agxbuf buf = {0};
      agxbprint(&buf, PFX1, graphName);
      char *name = agxbuse(&buf);
      dout = agsubg(dg, name, 1);
      out = agsubg(g, name, 1);
      agxbfree(&buf);
    }
    aginit(out, AGRAPH, "graphinfo", sizeof(graphinfo_t), true);
    GD_cc_subg(out) = true;
    Agnode_t *const dn = ND_dn(n);
    const long n_cnt = dfs(dg, dn, dout);
    unionNodes(dout, out);
    size_t e_cnt = 0;
    if (doEdges)
      e_cnt = graphviz_node_induce(out, out->root);
    if (doAll)
      subGInduce(g, out);
    gwrite(out);
    if (verbose)
      fprintf(stderr, " %7ld nodes %7" PRISIZE_T " edges\n", n_cnt, e_cnt);
    return 0;
  }

  size_t c_cnt = 0;
  for (Agnode_t *dn = agfstnode(dg); dn; dn = agnxtnode(dg, dn)) {
    if (Node_mark(dn))
      continue;
    {
      agxbuf buf = {0};
      agxbprint(&buf, PFX2, graphName, c_cnt);
      char *name = agxbuse(&buf);
      dout = agsubg(dg, name, 1);
      out = agsubg(g, name, 1);
      agxbfree(&buf);
    }
    aginit(out, AGRAPH, "graphinfo", sizeof(graphinfo_t), true);
    GD_cc_subg(out) = true;
    const long n_cnt = dfs(dg, dn, dout);
    unionNodes(dout, out);
    size_t e_cnt = 0;
    if (doEdges)
      e_cnt = graphviz_node_induce(out, out->root);
    if (printMode == EXTERNAL) {
      if (doAll)
        subGInduce(g, out);
      gwrite(out);
    } else if (printMode == EXTRACT) {
      if (x_mode == BY_INDEX) {
        if (x_index < 0 || (size_t)x_index <= c_cnt) {
          extracted = true;
          if (doAll)
            subGInduce(g, out);
          gwrite(out);
          if (x_final >= 0 && c_cnt == (size_t)x_final)
            return 0;
        }
      } else if (x_mode == BY_SIZE) {
        int sz = agnnodes(out);
        if (x_index <= sz && (x_final == -1 || sz <= x_final)) {
          extracted = true;
          if (doAll)
            subGInduce(g, out);
          gwrite(out);
        }
      }
    }
    if (printMode != INTERNAL)
      agdelete(g, out);
    agdelete(dg, dout);
    if (verbose)
      fprintf(stderr, "(%4" PRISIZE_T ") %7ld nodes %7" PRISIZE_T " edges\n",
              c_cnt, n_cnt, e_cnt);
    c_cnt++;
  }
  if (printMode == EXTRACT && !extracted && x_mode == BY_INDEX) {
    fprintf(stderr, "ccomps: component %d not found in graph %s - ignored\n",
            x_index, agnameof(g));
    return 1;
  }

  if (sorted)
    printSorted(g, c_cnt);
  else if (printMode == INTERNAL)
    gwrite(g);

  if (verbose)
    fprintf(stderr,
            "       %7d nodes %7d edges %7" PRISIZE_T " components %s\n",
            agnnodes(g), agnedges(g), c_cnt, agnameof(g));

  agclose(dg);

  return c_cnt ? 1 : 0;
}

static void bindGraphinfo(Agraph_t *g) {
  aginit(g, AGRAPH, "graphinfo", sizeof(graphinfo_t), true);
  for (Agraph_t *subg = agfstsubg(g); subg; subg = agnxtsubg(subg)) {
    bindGraphinfo(subg);
  }
}

/// return 0 if graph is connected
static int process(Agraph_t *g, char *graphName) {
  Agraph_t *out;
  bool extracted = false;

  aginit(g, AGNODE, "nodeinfo", sizeof(nodeinfo_t), true);
  bindGraphinfo(g);

  if (useClusters)
    return processClusters(g, graphName);

  if (x_node) {
    Agnode_t *const n = agfindnode(g, x_node);
    if (!n) {
      fprintf(stderr, "ccomps: node %s not found in graph %s - ignored\n",
              x_node, agnameof(g));
      return 1;
    }
    {
      agxbuf name = {0};
      agxbprint(&name, PFX1, graphName);
      out = agsubg(g, agxbuse(&name), 1);
      agxbfree(&name);
    }
    aginit(out, AGRAPH, "graphinfo", sizeof(graphinfo_t), true);
    GD_cc_subg(out) = true;
    const long n_cnt = dfs(g, n, out);
    size_t e_cnt = 0;
    if (doEdges)
      e_cnt = graphviz_node_induce(out, out->root);
    if (doAll)
      subGInduce(g, out);
    gwrite(out);
    if (verbose)
      fprintf(stderr, " %7ld nodes %7" PRISIZE_T " edges\n", n_cnt, e_cnt);
    return 0;
  }

  size_t c_cnt = 0;
  for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
    if (Node_mark(n))
      continue;
    {
      agxbuf name = {0};
      agxbprint(&name, PFX2, graphName, c_cnt);
      out = agsubg(g, agxbuse(&name), 1);
      agxbfree(&name);
    }
    aginit(out, AGRAPH, "graphinfo", sizeof(graphinfo_t), true);
    GD_cc_subg(out) = true;
    const long n_cnt = dfs(g, n, out);
    size_t e_cnt = 0;
    if (doEdges)
      e_cnt = graphviz_node_induce(out, out->root);
    if (printMode == EXTERNAL) {
      if (doAll)
        subGInduce(g, out);
      gwrite(out);
    } else if (printMode == EXTRACT) {
      if (x_mode == BY_INDEX) {
        if (x_index < 0 || (size_t)x_index <= c_cnt) {
          extracted = true;
          if (doAll)
            subGInduce(g, out);
          gwrite(out);
          if (x_final >= 0 && c_cnt == (size_t)x_final)
            return 0;
        }
      } else if (x_mode == BY_SIZE) {
        int sz = agnnodes(out);
        if (x_index <= sz && (x_final == -1 || sz <= x_final)) {
          extracted = true;
          if (doAll)
            subGInduce(g, out);
          gwrite(out);
        }
      }
    }
    if (printMode != INTERNAL)
      agdelete(g, out);
    if (verbose)
      fprintf(stderr, "(%4" PRISIZE_T ") %7ld nodes %7" PRISIZE_T " edges\n",
              c_cnt, n_cnt, e_cnt);
    c_cnt++;
  }
  if (printMode == EXTRACT && !extracted && x_mode == BY_INDEX) {
    fprintf(stderr, "ccomps: component %d not found in graph %s - ignored\n",
            x_index, agnameof(g));
    return 1;
  }

  if (sorted)
    printSorted(g, c_cnt);
  else if (printMode == INTERNAL)
    gwrite(g);

  if (verbose)
    fprintf(stderr,
            "       %7d nodes %7d edges %7" PRISIZE_T " components %s\n",
            agnnodes(g), agnedges(g), c_cnt, agnameof(g));
  return c_cnt > 1;
}

/* If the graph is anonymous, its name starts with '%'.
 * If we use this as the prefix for subgraphs, they will not be
 * emitted in agwrite() because cgraph assumes these were anonymous
 * structural subgraphs all of whose properties are attached directly
 * to the nodes and edges involved.
 *
 * This function checks for an initial '%' and if found, prepends '_'
 * to the graph name.
 * NB: static buffer is used.
 */
static char *chkGraphName(Agraph_t *g) {
  static agxbuf buf;
  char *s = agnameof(g);

  if (*s != '%')
    return s;
  agxbprint(&buf, "_%s", s);

  return agxbuse(&buf);
}

int main(int argc, char *argv[]) {
  Agraph_t *g;
  ingraph_state ig;
  int r = 0;
  init(argc, argv);
  newIngraph(&ig, Inputs);

  while ((g = nextGraph(&ig)) != 0) {
    r += process(g, chkGraphName(g));
    agclose(g);
  }

  node_stack_free(&Stk);

  graphviz_exit(r);
}
