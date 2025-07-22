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
 * gvpr: graph pattern recognizer
 *
 * Written by Emden Gansner
 */

#include <gvpr/gvpr.h>
#include <util/exit.h>

int
main (int argc, char* argv[])
{
    gvpropts opts;
    opts.ingraphs = 0;
    opts.out = 0;
    opts.err = 0;
    opts.flags = GV_USE_EXIT;
    opts.bindings = 0;
    
    graphviz_exit(gvpr(argc, argv, &opts));
}

/**
 * @dir cmd/gvpr
 * @brief graph pattern scanning and processing language
 */
