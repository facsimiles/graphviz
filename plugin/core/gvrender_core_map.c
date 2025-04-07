/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <stdlib.h>

#include <gvc/gvplugin_render.h>
#include <gvc/gvplugin_device.h>
#include <gvc/gvio.h>
#include <util/unreachable.h>
#include <util/xml.h>

enum { FORMAT_IMAP, FORMAT_ISMAP, FORMAT_CMAP, FORMAT_CMAPX, };

/// wrap `gvputs` to offer a `void *` first parameter
static int gvputs_wrapper(void *state, const char *s) {
  return gvputs(state, s);
}

// wrapper around `gv_xml_escape` to set flags for URL escaping
static void xml_url_puts(GVJ_t *job, const char *s) {
  const xml_flags_t flags = {0};
  (void)gv_xml_escape(s, flags, gvputs_wrapper, job);
}

static void map_output_shape(GVJ_t *job, map_shape_t map_shape, pointf *AF, size_t nump,
                char* url, char *tooltip, char *target, char *id)
{
    if (!AF || !nump)
	return;

    if (job->render.id == FORMAT_IMAP && url && url[0]) {
        switch (map_shape) {
        case MAP_RECTANGLE:
	    /* Y_GOES_DOWN so need UL to LR */
            gvprintf(job, "rect %s %.0f,%.0f %.0f,%.0f\n", url, AF[0].x,
                     AF[1].y, AF[1].x, AF[0].y);
            break;
        case MAP_CIRCLE:
            gvprintf(job, "circle %s %.0f,%.0f,%.0f\n", url, AF[0].x, AF[0].y,
                     AF[1].x - AF[0].x);
            break;
        case MAP_POLYGON:
            gvprintf(job, "poly %s", url);
            for (size_t i = 0; i < nump; i++) {
                gvprintf(job, " %.0f,%.0f", AF[i].x, AF[i].y);
            }
            gvputs(job, "\n");
            break;
        default:
            UNREACHABLE();
        }

    } else if (job->render.id == FORMAT_ISMAP && url && url[0]) {
        switch (map_shape) {
        case MAP_RECTANGLE:
	    /* Y_GOES_DOWN so need UL to LR */
            gvprintf(job, "rectangle (%.0f,%.0f) (%.0f,%.0f) %s %s\n",
                     AF[0].x, AF[1].y, AF[1].x, AF[0].y, url, tooltip);
	    break;
        default:
            UNREACHABLE();
        }

    } else if (job->render.id == FORMAT_CMAP || job->render.id == FORMAT_CMAPX) {
        switch (map_shape) {
        case MAP_CIRCLE:
            gvputs(job, "<area shape=\"circle\"");
            break;
        case MAP_RECTANGLE:
            gvputs(job, "<area shape=\"rect\"");
            break;
        case MAP_POLYGON:
            gvputs(job, "<area shape=\"poly\"");
            break;
        default:
            UNREACHABLE();
        }
        if (id && id[0]) {
            gvputs(job, " id=\"");
	    xml_url_puts(job, id);
	    gvputs(job, "\"");
	}
        if (url && url[0]) {
            gvputs(job, " href=\"");
	    xml_url_puts(job, url);
	    gvputs(job, "\"");
	}
        if (target && target[0]) {
            gvputs(job, " target=\"");
	    gvputs_xml(job, target);
	    gvputs(job, "\"");
	}
        if (tooltip && tooltip[0]) {
            gvputs(job, " title=\"");
	    gvputs_xml(job, tooltip);
	    gvputs(job, "\"");
	}
        // FIXME:
        //  - https://gitlab.com/graphviz/graphviz/-/issues/265
        //  - https://gitlab.com/graphviz/graphviz/-/issues/2220
        gvputs(job, " alt=\"\"");

        gvputs(job, " coords=\"");
        switch (map_shape) {
        case MAP_CIRCLE:
            gvprintf(job, "%.0f,%.0f,%.0f", AF[0].x, AF[0].y,
                     AF[1].x - AF[0].x);
            break;
        case MAP_RECTANGLE:
	    /* Y_GOES_DOWN so need UL to LR */
            gvprintf(job, "%.0f,%.0f,%.0f,%.0f", AF[0].x, AF[1].y, AF[1].x,
                     AF[0].y);
            break;
        case MAP_POLYGON:
            gvprintf(job, "%.0f,%.0f", AF[0].x, AF[0].y);
            for (size_t i = 1; i < nump; i++) {
                gvprintf(job, ",%.0f,%.0f", AF[i].x, AF[i].y);
            }
            break;
        default:
            break;
        }
        if (job->render.id == FORMAT_CMAPX)
            gvputs(job, "\"/>\n");
	else
            gvputs(job, "\">\n");
    }
}

static void map_begin_page(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    switch (job->render.id) {
    case FORMAT_IMAP:
        gvputs(job, "base referer\n");
        if (obj->url && obj->url[0]) {
	    gvputs(job, "default ");
	    gvputs_xml(job, obj->url);
	    gvputs(job, "\n");
	}
        break;
    case FORMAT_ISMAP:
        if (obj->url && obj->url[0]) {
	    gvputs(job, "default ");
	    gvputs_xml(job, obj->url);
	    gvputs(job, " ");
	    gvputs_xml(job, agnameof(obj->u.g));
	    gvputs(job, "\n");
	}
        break;
    case FORMAT_CMAPX:
	gvputs(job, "<map id=\"");
	gvputs_xml(job, agnameof(obj->u.g));
	gvputs(job, "\" name=\"");
	gvputs_xml(job, agnameof(obj->u.g));
	gvputs(job, "\">\n");
        break;
    default:
	break;
    }
}

static void map_end_page(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    switch (job->render.id) {
    case FORMAT_CMAP:
	map_output_shape(job, obj->url_map_shape,
		obj->url_map_p,obj->url_map_n,
		obj->url, obj->tooltip, obj->target, obj->id);
	break;
    case FORMAT_CMAPX:
	map_output_shape(job, obj->url_map_shape,
		obj->url_map_p,obj->url_map_n,
		obj->url, obj->tooltip, obj->target, obj->id);
        gvputs(job, "</map>\n");
	break;
    default:
	break;
    }
}

static void map_begin_anchor(GVJ_t * job, char *url, char *tooltip, char *target, char *id)
{
    obj_state_t *obj = job->obj;

    map_output_shape(job, obj->url_map_shape,
		obj->url_map_p, obj->url_map_n, 
		url, tooltip, target, id);
}

static gvrender_engine_t map_engine = {
    0,				/* map_begin_job */
    0,				/* map_end_job */
    0,				/* map_begin_graph */
    0,				/* map_end_graph */
    0,				/* map_begin_layer */
    0,				/* map_end_layer */
    map_begin_page,
    map_end_page,
    0,				/* map_begin_cluster */
    0,				/* map_end_cluster */
    0,				/* map_begin_nodes */
    0,				/* map_end_nodes */
    0,				/* map_begin_edges */
    0,				/* map_end_edges */
    0,				/* map_begin_node */
    0,				/* map_end_node */
    0,				/* map_begin_edge */
    0,				/* map_end_edge */
    map_begin_anchor,
    0,				/* map_end_anchor */
    0,				/* map_begin_label */
    0,				/* map_end_label */
    0,				/* map_textpara */
    0,				/* map_resolve_color */
    0,				/* map_ellipse */
    0,				/* map_polygon */
    0,				/* map_bezier */
    0,				/* map_polyline */
    0,				/* map_comment */
    0,				/* map_library_shape */
};

static gvrender_features_t render_features_map = {
    EMIT_CLUSTERS_LAST
        | GVRENDER_Y_GOES_DOWN
	| GVRENDER_DOES_MAPS
	| GVRENDER_DOES_LABELS
	| GVRENDER_DOES_TOOLTIPS
	| GVRENDER_DOES_TARGETS
	| GVRENDER_DOES_MAP_RECTANGLE, /* flags */
    4.,                         /* default pad - graph units */
    NULL,			/* knowncolors */
    0,				/* sizeof knowncolors */
    0,				/* color_type */
};

static gvdevice_features_t device_features_map = {
    GVRENDER_DOES_MAP_CIRCLE
	| GVRENDER_DOES_MAP_POLYGON, /* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},			/* default dpi */
};

static gvdevice_features_t device_features_map_nopoly = {
    0,				/* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},			/* default dpi */
};

gvplugin_installed_t gvrender_map_types[] = {
    {FORMAT_ISMAP, "map", 1, &map_engine, &render_features_map},
    {0, NULL, 0, NULL, NULL}
};

gvplugin_installed_t gvdevice_map_types[] = {
    {FORMAT_ISMAP, "ismap:map", 1, NULL, &device_features_map_nopoly},
    {FORMAT_CMAP, "cmap:map", 1, NULL, &device_features_map},
    {FORMAT_IMAP, "imap:map", 1, NULL, &device_features_map},
    {FORMAT_CMAPX, "cmapx:map", 1, NULL, &device_features_map},
    {FORMAT_IMAP, "imap_np:map", 1, NULL, &device_features_map_nopoly},
    {FORMAT_CMAPX, "cmapx_np:map", 1, NULL, &device_features_map_nopoly},
    {0, NULL, 0, NULL, NULL}
};
