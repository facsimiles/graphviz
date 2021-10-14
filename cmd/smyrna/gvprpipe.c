/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "smyrnadefs.h"
#include "gvprpipe.h"
#include <common/const.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include "draw.h"
#include "gui.h"
#include "topviewsettings.h"
#include <viewport.h>

#include <gvpr/gvpr.h>

static ssize_t outfn(void *sp, const char *buf, size_t nbyte, void *dp)
{
    (void)sp;
    (void)dp;

    append_textview((GtkTextView *)
		    glade_xml_get_widget(xml, "gvprtextoutput"), buf,
		    nbyte);
    append_textview((GtkTextView *)
		    glade_xml_get_widget(xml, "mainconsole"), buf, nbyte);
    return (ssize_t)nbyte;
}

int run_gvpr(Agraph_t * srcGraph, int argc, char *argv[])
{
    int i, rv = 1;
    gvpropts opts;
    Agraph_t *gs[2];
    static int count;
    char buf[SMALLBUF];

    gs[0] = srcGraph;
    gs[1] = 0;
    memset (&opts, 0, sizeof(opts));
    opts.ingraphs = gs;
    opts.out = outfn;
    opts.err = outfn;
    opts.flags = GV_USE_OUTGRAPH;

    rv = gvpr(argc, argv, &opts);

    if (rv) {			/* error */
	fprintf(stderr, "Error in gvpr\n");
    } else if (opts.n_outgraphs) 
    {
	refreshViewport(0);
	snprintf(buf, sizeof(buf), "<%d>", ++count);
	if (opts.outgraphs[0] != view->g[view->activeGraph])
	    add_graph_to_viewport(opts.outgraphs[0], buf);
	if (opts.n_outgraphs > 1)
	    fprintf(stderr, "Warning: multiple output graphs-discarded\n");
	for (i = 1; i < opts.n_outgraphs; i++) {
	    agclose(opts.outgraphs[i]);
	}
    } else 
    { 
	updateRecord (srcGraph);
        update_graph_from_settings(srcGraph);
    }
    return rv;
}
