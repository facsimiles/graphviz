/**
 * @file
 * @brief make directed graph acyclic
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

#include "config.h"

#include <stdbool.h>
#include <stdio.h>

#include <stdlib.h>
#include <cgraph/cgraph.h>
#include "openFile.h"
#include <util/exit.h>
#include <util/prisize_t.h>
#include <util/unreachable.h>

#define graphName(g) (agnameof(g))

#include <getopt.h>

static FILE *inFile;

typedef graphviz_acyclic_options_t opts_t;

static char *cmd;

static char *useString = "Usage: %s [-nv?] [-o outfile] <file>\n\
  -o <file> - put output in <file>\n\
  -n        - do not output graph\n\
  -v        - verbose\n\
  -?        - print usage\n";

static void usage(int v)
{
    fprintf(stderr, useString, cmd);
    graphviz_exit(v);
}

static void init(opts_t *opts, int argc, char *argv[]) {
    int c;

    cmd = argv[0];
    opterr = 0;
    while ((c = getopt(argc, argv, ":vno:")) != -1)
	switch (c) {
	case 'o':
	    if (opts->outFile != NULL)
		fclose(opts->outFile);
	    opts->outFile = openFile(argv[0], optarg, "w");
	    break;
	case 'n':
	    opts->doWrite = false;
	    break;
	case 'v':
	    opts->Verbose = true;
	    break;
	case '?':
	    if (optopt == '?')
		usage(0);
	    else {
		fprintf(stderr, "%s: option -%c unrecognized\n", cmd,
			optopt);
		usage(-1);
	    }
	    break;
	case ':':
	    fprintf(stderr, "%s: missing argument for option -%c\n",
		    cmd, optopt);
	    usage(-1);
	    break;
	default:
	    UNREACHABLE();
	}
    if (optind < argc) {
	inFile = openFile(argv[0], argv[optind], "r");
    } else
	inFile = stdin;
    if (!opts->outFile)
	opts->outFile = stdout;
}

int main(int argc, char *argv[])
{
    Agraph_t *g;
    int rv = 0;
    opts_t opts = {.doWrite = true};
    size_t num_rev = 0;

    init(&opts, argc, argv);

    if ((g = agread(inFile, NULL)) != NULL) {
	if (agisdirected (g)) {
	    rv |= graphviz_acyclic(g, &opts, &num_rev);
	    if (opts.Verbose) {
		if (rv)
		    fprintf(stderr, "Graph \"%s\" has cycles; %" PRISIZE_T " reversed edges\n",
		            graphName(g), num_rev);
		else
		    fprintf(stderr, "Graph \"%s\" is acyclic\n", graphName(g));
	    }
	} else {
	    rv = -1;
	    if (opts.Verbose)
		fprintf(stderr, "Graph \"%s\" is undirected\n", graphName(g));
	}
	graphviz_exit(rv);
    } else
	graphviz_exit(-1);
}
