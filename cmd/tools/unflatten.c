/**
 * @file
 * @brief adjust directed graphs to improve layout aspect ratio
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

#include    <stdbool.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <cgraph/cgraph.h>
#include    <cgraph/ingraphs.h>

#include <getopt.h>
#include "openFile.h"
#include <util/exit.h>
#include <util/unreachable.h>

typedef graphviz_unflatten_options_t opts_t;

static FILE *outFile;
static char *cmd;

static char *useString =
    "Usage: %s [-f?] [-l <M>] [-c <N>] [-o <outfile>] <files>\n\
  -o <outfile> - put output in <outfile>\n\
  -f           - adjust immediate fanout chains\n\
  -l <M>       - stagger length of leaf edges between [1,<M>]\n\
  -c <N>       - put disconnected nodes in chains of length <N>\n\
  -?           - print usage\n";

static void usage(int v)
{
    fprintf(stderr, useString, cmd);
    graphviz_exit(v);
}

static char **scanargs(opts_t *opts, int argc, char **argv) {
    int c, ival;

    cmd = argv[0];
    opterr = 0;
    while ((c = getopt(argc, argv, ":fl:c:o:")) != -1) {
	switch (c) {
	case 'f':
	    opts->Do_fans = true;
	    break;
	case 'l':
	    ival = atoi(optarg);
	    if (ival > 0)
		opts->MaxMinlen = ival;
	    break;
	case 'c':
	    ival = atoi(optarg);
	    if (ival > 0)
		opts->ChainLimit = ival;
	    break;
	case 'o':
	    if (outFile != NULL)
		fclose(outFile);
	    outFile = openFile(cmd, optarg, "w");
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
    }
    if (opts->Do_fans && opts->MaxMinlen < 1)
	fprintf(stderr, "%s: Warning: -f requires -l flag\n", cmd);
    argv += optind;
    argc -= optind;

    if (!outFile)
	outFile = stdout;
    if (argc)
	return argv;
    else
	return 0;
}

int main(int argc, char **argv)
{
    Agraph_t *g;
    ingraph_state ig;
    char **files;
    opts_t opts = {0};

    files = scanargs(&opts, argc, argv);
    newIngraph(&ig, files);
    while ((g = nextGraph(&ig))) {
	graphviz_unflatten(g, &opts);
	agwrite(g, outFile);
    }
    graphviz_exit(0);
}
