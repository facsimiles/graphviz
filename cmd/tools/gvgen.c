/**
 * @file
 * @brief generate graphs
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
 * Written by Emden Gansner
 */

#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>
#include "graph_generator.h"
#include "openFile.h"
#include <util/exit.h>

typedef enum { unknown, grid, circle, complete, completeb, 
    path, tree, torus, cylinder, mobius, randomg, randomt, ball,
    sierpinski, hypercube, star, wheel, trimesh
} GraphType;

typedef struct {
    unsigned graphSize1;
    unsigned graphSize2;
    unsigned cnt;
    unsigned parm1;
    unsigned parm2;
    int Verbose;
    int isPartial;
    int foldVal;
    int directed;
    FILE *outfile;
    char* pfx;
    char* name;
    unsigned seed; ///< initial state for random number generator
} opts_t;

static char *cmd;

static char *Usage = "Usage: %s [-dv?] [options]\n\
 -c<n>         : cycle \n\
 -C<x,y>       : cylinder \n\
 -g[f]<h,w>    : grid (folded if f is used)\n\
 -G[f]<h,w>    : partial grid (folded if f is used)\n\
 -h<x>         : hypercube \n\
 -k<x>         : complete \n\
 -b<x,y>       : complete bipartite\n\
 -B<x,y>       : ball\n\
 -i<n>         : generate <n> random\n\
 -m<x>         : triangular mesh\n\
 -M<x,y>       : x by y Moebius strip\n\
 -n<prefix>    : use <prefix> in node names (\"\")\n\
 -N<name>      : use <name> for the graph (\"\")\n\
 -o<outfile>   : put output in <outfile> (stdout)\n\
 -p<x>         : path \n\
 -r<x>,<n>     : random graph\n\
 -R<n>         : random rooted tree on <n> vertices\n\
 -s<x>         : star\n\
 -S<x>         : 2D sierpinski\n\
 -S<x>,<d>     : <d>D sierpinski (<d> = 2,3)\n\
 -t<x>         : binary tree \n\
 -t<x>,<n>     : n-ary tree \n\
 -T<x,y>       : torus \n\
 -T<x,y,t1,t2> : twisted torus \n\
 -u<seed>      : state for random number generation\n\
 -w<x>         : wheel\n\
 -d            : directed graph\n\
 -v            : verbose mode\n\
 -?            : print usage\n";

static void usage(int v)
{
    fprintf(v ? stderr : stdout, Usage, cmd);
    graphviz_exit(v);
}

static void errexit(int opt) {
    fprintf(stderr, "in flag -%c\n", (char)opt);
    usage(1);
}

/* readPos:
 * Read and return a single unsigned int from s, guaranteed to be >= 1.
 * A pointer to the next available character from s is stored in e.
 * Return 0 on error.
 */
static unsigned readPos(char *s, char **e) {
    static const unsigned MIN = 1;

    const unsigned long d = strtoul(s, e, 10);
    if (s == *e || d > UINT_MAX) {
	fprintf(stderr, "ill-formed integer \"%s\" ", s);
	return 0;
    }
    if (d < MIN) {
	fprintf(stderr, "integer \"%s\" less than %d", s, MIN);
	return 0;
    }
    return (unsigned)d;
}

/* readOne:
 * Return non-zero on error.
 */
static int readOne(char *s, unsigned *ip) {
    const unsigned d = readPos(s, &(char *){NULL});
    if (d > 0) {
	*ip = d;
	return 0;
    }
    return -1;
}

/* setOne:
 * Return non-zero on error.
 */
static int setOne(char *s, opts_t* opts)
{
  return readOne(s, &opts->graphSize1);
}

/* setTwo:
 * Return non-zero on error.
 */
static int setTwo(char *s, opts_t* opts)
{
    char *next;

    unsigned d = readPos(s, &next);
    if (d == 0)
	return -1;
    opts->graphSize1 = d;

    if (*next != ',') {
	fprintf(stderr, "ill-formed int pair \"%s\" ", s);
	return -1;
    }

    s = next + 1;
    d = readPos(s, &(char *){NULL});
    if (d > 1) {
	opts->graphSize2 = d;
	return 0;
    }
    return -1;
}

/* setTwoTwoOpt:
 * Read 2 numbers
 * Read 2 more optional numbers
 * Return non-zero on error.
 */
static int setTwoTwoOpt(char *s, opts_t *opts, unsigned dflt) {
    char *next;

    unsigned d = readPos(s, &next);
    if (d == 0)
	return -1;
    opts->graphSize1 = d;

    if (*next != ',') {
	fprintf(stderr, "ill-formed int pair \"%s\" ", s);
	return -1;
    }

    s = next + 1;
    d = readPos(s, &next);
    if (d == 0) {
	return 0;
    }
    opts->graphSize2 = d;

    if (*next != ',') {
	opts->parm1 = opts->parm2 = dflt;
	return 0;
    }

    s = next + 1;
    d = readPos(s, &next);
    if (d == 0)
	return -1;
    opts->parm1 = d;

    if (*next != ',') {
	opts->parm2 = dflt;
	return 0;
    }

    s = next + 1;
    return readOne(s, &opts->parm2);
}

/* setTwoOpt:
 * Return non-zero on error.
 */
static int setTwoOpt(char *s, opts_t *opts, unsigned dflt) {
    char *next;

    unsigned d = readPos(s, &next);
    if (d == 0)
	return -1;
    opts->graphSize1 = d;

    if (*next != ',') {
	opts->graphSize2 = dflt;
	return 0;
    }

    s = next + 1;
    d = readPos(s, &(char *){NULL});
    if (d > 1) {
	opts->graphSize2 = d;
	return 0;
    }
    return -1;
}

static char* setFold(char *s, opts_t* opts)
{
    char *next;

    if (*s == 'f') {
	next = s+1;
	opts->foldVal = 1;
    }
    else
	next = s;

    return next;
}

static char *optList = ":i:M:m:n:N:c:C:dg:G:h:k:b:B:o:p:r:R:s:S:X:t:T:u:vw:";

static GraphType init(int argc, char *argv[], opts_t* opts)
{
    int c;
    GraphType graphType = unknown;

    cmd = argv[0];
    opterr = 0;
    while ((c = getopt(argc, argv, optList)) != -1) {
	switch (c) {
	case 'c':
	    graphType = circle;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'C':
	    graphType = cylinder;
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'M':
	    graphType = mobius;
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'd':
	    opts->directed = 1;
	    break;
	case 'G':
	    opts->isPartial = 1;
	    // fall through
	case 'g':
	    graphType = grid;
	    optarg = setFold (optarg, opts);
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'h':
	    graphType = hypercube;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'k':
	    graphType = complete;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'b':
	    graphType = completeb;
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'B':
	    graphType = ball;
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'm':
	    graphType = trimesh;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'r':
	    graphType = randomg;
	    if (setTwo(optarg, opts))
		errexit(c);
	    break;
	case 'R':
	    graphType = randomt;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'n':
	    opts->pfx = optarg;
	    break;
	case 'N':
	    opts->name = optarg;
	    break;
	case 'o':
	    opts->outfile = openFile(cmd, optarg, "w");
	    break;
	case 'p':
	    graphType = path;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 'S':
	    graphType = sierpinski;
	    if (setTwoOpt(optarg, opts, 2))
		errexit(c);
	    if (opts->graphSize2 > 3) {
		fprintf(stderr, "%uD Sierpinski not implemented - use 2 or 3 ",
		        opts->graphSize2);
		errexit(c);
	    }
	    break;
	case 's':
	    graphType = star;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case 't':
	    graphType = tree;
	    if (setTwoOpt(optarg, opts, 2))
		errexit(c);
	    break;
	case 'T':
	    graphType = torus;
	    if (setTwoTwoOpt(optarg, opts, 0))
		errexit(c);
	    break;
	case 'i':
	    if (readOne(optarg, &opts->cnt))
		errexit(c);
	    break;
	case 'u':
	    if (readOne(optarg, &opts->seed))
		errexit(c);
	    break;
	case 'v':
	    opts->Verbose = 1;
	    break;
	case 'w':
	    graphType = wheel;
	    if (setOne(optarg, opts))
		errexit(c);
	    break;
	case '?':
	    if (optopt == '?')
		usage(0);
	    else
		fprintf(stderr, "Unrecognized flag \"-%c\" - ignored\n",
			optopt);
	    break;
	default:
	    fprintf(stderr, "Unexpected error\n");
	    usage(EXIT_FAILURE);
	}
    }

    argc -= optind;
    argv += optind;
    if (!opts->outfile)
	opts->outfile = stdout;
    if (graphType == unknown) {
	fprintf(stderr, "Graph type not set\n");
	usage(1);
    }

    return graphType;
}

static opts_t opts;

static void dirfn(unsigned t, unsigned h) {
    if (h > 0)
	fprintf(opts.outfile, "  %s%u -> %s%u\n", opts.pfx, t, opts.pfx, h);
    else
	fprintf(opts.outfile, "  %s%u\n", opts.pfx, t);
}

static void undirfn(unsigned t, unsigned h) {
    if (h > 0)
	fprintf(opts.outfile, "  %s%u -- %s%u\n", opts.pfx, t, opts.pfx, h);
    else
	fprintf(opts.outfile, "  %s%u\n", opts.pfx, t);
}

static void
closeOpen (void)
{
    if (opts.directed)
	fprintf(opts.outfile, "}\ndigraph {\n");
    else
	fprintf(opts.outfile, "}\ngraph {\n");
}

int main(int argc, char *argv[])
{
    GraphType graphType;
    edgefn ef;

    opts.pfx = "";
    opts.name = "";
    opts.cnt = 1;
    opts.seed = (unsigned)time(0);
    graphType = init(argc, argv, &opts);
    if (opts.directed) {
	fprintf(opts.outfile, "digraph %s{\n", opts.name);
	ef = dirfn;
    }
    else {
	fprintf(opts.outfile, "graph %s{\n", opts.name);
	ef = undirfn;
    }

    // seed the random number generator
    srand(opts.seed);

    switch (graphType) {
    case grid:
	makeSquareGrid(opts.graphSize1, opts.graphSize2,
		       opts.foldVal, opts.isPartial, ef);
	break;
    case circle:
	makeCircle(opts.graphSize1, ef);
	break;
    case path:
	makePath(opts.graphSize1, ef);
	break;
    case tree:
	if (opts.graphSize2 == 2)
	    makeBinaryTree(opts.graphSize1, ef);
	else
	    makeTree(opts.graphSize1, opts.graphSize2, ef);
	break;
    case trimesh:
	makeTriMesh(opts.graphSize1, ef);
	break;
    case ball:
	makeBall(opts.graphSize1, opts.graphSize2, ef);
	break;
    case torus:
	if (opts.parm1 == 0 && opts.parm2 == 0)
	    makeTorus(opts.graphSize1, opts.graphSize2, ef);
	else
	    makeTwistedTorus(opts.graphSize1, opts.graphSize2, opts.parm1, opts.parm2, ef);
	break;
    case cylinder:
	makeCylinder(opts.graphSize1, opts.graphSize2, ef);
	break;
    case mobius:
	makeMobius(opts.graphSize1, opts.graphSize2, ef);
	break;
    case sierpinski:
	if (opts.graphSize2 == 2)
	    makeSierpinski(opts.graphSize1, ef);
	else
	    makeTetrix(opts.graphSize1, ef);
	break;
    case complete:
	makeComplete(opts.graphSize1, ef);
	break;
    case randomg:
	makeRandom (opts.graphSize1, opts.graphSize2, ef);
	break;
    case randomt:
	{
	    treegen_t* tg = makeTreeGen (opts.graphSize1);
	    for (unsigned i = 1; i <= opts.cnt; i++) {
		makeRandomTree (tg, ef);
		if (i != opts.cnt) closeOpen ();
	    }
	    freeTreeGen (tg);
	}
	makeRandom (opts.graphSize1, opts.graphSize2, ef);
	break;
    case completeb:
	makeCompleteB(opts.graphSize1, opts.graphSize2, ef);
	break;
    case hypercube:
	makeHypercube(opts.graphSize1, ef);
	break;
    case star:
	makeStar(opts.graphSize1, ef);
	break;
    case wheel:
	makeWheel(opts.graphSize1, ef);
	break;
    default:
	/* can't happen */
	break;
    }
    fprintf(opts.outfile, "}\n");

    graphviz_exit(0);
}
