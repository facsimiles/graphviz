/**
 * @file
 * @brief
 * [transitive reduction](https://en.wikipedia.org/wiki/Transitive_reduction)
 * filter for directed graphs
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *************************************************************************/


/*
 * Written by Stephen North
 * Updated by Emden Gansner
 */

/*
 * reads a sequence of graphs on stdin, and writes their
 * transitive reduction
 */

#include <cgraph/cgraph.h>
#include <cgraph/ingraphs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <util/exit.h>
#include <util/unreachable.h>

static char **Files;
static char *CmdName;

typedef graphviz_tred_options_t opts_t;

static char *useString = "Usage: %s [-vr?] <files>\n\
  -o FILE - redirect output (default to stdout)\n\
  -v - verbose (to stderr)\n\
  -r - print removed edges to stderr\n\
  -? - print usage\n\
If no files are specified, stdin is used\n";

static void usage(int v)
{
    printf(useString, CmdName);
    graphviz_exit(v);
}

static void init(opts_t *opts, int argc, char *argv[]) {
    int c;

    CmdName = argv[0];
    opterr = 0;
    while ((c = getopt(argc, argv, "o:vr?")) != -1) {
	switch (c) {
	case 'o':
	    (void)fclose(opts->out);
	    opts->out = fopen(optarg, "w");
	    if (opts->out == NULL) {
		fprintf(stderr, "cannot open %s for writing\n",
			optarg);
		usage(1);
	    }
	    break;
	case 'v':
	    opts->Verbose = true;
	    break;
	case 'r':
        opts->PrintRemovedEdges = true;
        break;
	case '?':
	    if (optopt == '\0' || optopt == '?')
		usage(0);
	    else {
		fprintf(stderr, "%s: option -%c unrecognized\n",
			CmdName, optopt);
		usage(1);
	    }
	    break;
	default:
	    UNREACHABLE();
	}
    }
    argv += optind;
    argc -= optind;

    if (argc)
	Files = argv;
}

int main(int argc, char **argv)
{
    Agraph_t *g;
    ingraph_state ig;
    opts_t opts = {.out = stdout, .err = stderr};

    init(&opts, argc, argv);
    newIngraph(&ig, Files);

    while ((g = nextGraph(&ig)) != 0) {
	if (agisdirected(g))
	    graphviz_tred(g, &opts);
	agclose(g);
    }

    graphviz_exit(0);
}

