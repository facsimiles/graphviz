/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/* Comments on the SVG coordinate system (SN 8 Dec 2006):
   The initial <svg> element defines the SVG coordinate system so
   that the graphviz canvas (in units of points) fits the intended
   absolute size in inches.  After this, the situation should be
   that "px" = "pt" in SVG, so we can dispense with stating units.
   Also, the input units (such as fontsize) should be preserved
   without scaling in the output SVG (as long as the graph size
   was not constrained.)
 */

#include "config.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <common/macros.h>
#include <common/const.h>

#include <gvc/gvplugin_render.h>
#include <cgraph/agxbuf.h>
#include <common/utils.h>
#include <gvc/gvplugin_device.h>
#include <gvc/gvio.h>
#include <gvc/gvcint.h>
#include <cgraph/strcasecmp.h>

#define LOCALNAMEPREFIX		'%'

#ifndef EDGEALIGN
  #define EDGEALIGN 0
#endif

// optimized gvputs for string literals
#define GVPUTS(job, str) gvwrite((job), (str), sizeof(str) - 1)

typedef enum { FORMAT_SVG, FORMAT_SVGZ, } format_type;

/* SVG dash array */
static char *sdasharray = "5,2";
/* SVG dot array */
static char *sdotarray = "1,5";

static void svg_bzptarray(GVJ_t * job, pointf * A, int n)
{
    int i;
    char c;

    c = 'M';			/* first point */
#if EDGEALIGN
    if (A[0].x <= A[n-1].x) {
#endif
	for (i = 0; i < n; i++) {
	    gvwrite(job, &c, 1);
            gvprintdouble(job, A[i].x);
            GVPUTS(job, ",");
            gvprintdouble(job, -A[i].y);
	    if (i == 0)
		c = 'C';		/* second point */
	    else
		c = ' ';		/* remaining points */
	}
#if EDGEALIGN
    } else {
	for (i = n-1; i >= 0; i--) {
	    gvwrite(job, &c, 1);
            gvprintdouble(job, A[i].x);
            GVPUTS(job, ",");
            gvprintdouble(job, -A[i].y);
	    if (i == 0)
		c = 'C';		/* second point */
	    else
		c = ' ';		/* remaining points */
	}
    }
#endif
}

static void svg_print_id_class(GVJ_t * job, char* id, char* idx, char* kind, void* obj)
{
    char* str;

    GVPUTS(job, "<g id=\"");
    gvputs_xml(job, id);
    if (idx) {
	GVPUTS(job, "_");
	gvputs_xml(job, idx);
    }
    gvprintf(job, "\" class=\"%s", kind);
    if ((str = agget(obj, "class")) && *str) {
	GVPUTS(job, " ");
	gvputs_xml(job, str);
    }
    GVPUTS(job, "\"");
}

static void svg_print_color(GVJ_t * job, gvcolor_t color)
{
    switch (color.type) {
    case COLOR_STRING:
	gvputs(job, color.u.string);
	break;
    case RGBA_BYTE:
	if (color.u.rgba[3] == 0)	/* transparent */
	    GVPUTS(job, "transparent");
	else
	    gvprintf(job, "#%02x%02x%02x",
		     color.u.rgba[0], color.u.rgba[1], color.u.rgba[2]);
	break;
    default:
	assert(0);		/* internal error */
    }
}

static void svg_grstyle(GVJ_t * job, int filled, int gid)
{
    obj_state_t *obj = job->obj;

    GVPUTS(job, " fill=\"");
    if (filled == GRADIENT) {
	gvprintf(job, "url(#l_%d)", gid);
    } else if (filled == RGRADIENT) {
	gvprintf(job, "url(#r_%d)", gid);
    } else if (filled) {
	svg_print_color(job, obj->fillcolor);
	if (obj->fillcolor.type == RGBA_BYTE
	    && obj->fillcolor.u.rgba[3] > 0
	    && obj->fillcolor.u.rgba[3] < 255)
	    gvprintf(job, "\" fill-opacity=\"%f",
		     ((float) obj->fillcolor.u.rgba[3] / 255.0));
    } else {
	GVPUTS(job, "none");
    }
    GVPUTS(job, "\" stroke=\"");
    svg_print_color(job, obj->pencolor);
    if (obj->penwidth != PENWIDTH_NORMAL) {
	GVPUTS(job, "\" stroke-width=\"");
        gvprintdouble(job, obj->penwidth);
    }
    if (obj->pen == PEN_DASHED) {
	gvprintf(job, "\" stroke-dasharray=\"%s", sdasharray);
    } else if (obj->pen == PEN_DOTTED) {
	gvprintf(job, "\" stroke-dasharray=\"%s", sdotarray);
    }
    if (obj->pencolor.type == RGBA_BYTE && obj->pencolor.u.rgba[3] > 0
	&& obj->pencolor.u.rgba[3] < 255)
	gvprintf(job, "\" stroke-opacity=\"%f",
		 ((float) obj->pencolor.u.rgba[3] / 255.0));

    GVPUTS(job, "\"");
}

static void svg_comment(GVJ_t * job, char *str)
{
    GVPUTS(job, "<!-- ");
    gvputs_xml(job, str);
    GVPUTS(job, " -->\n");
}

static void svg_begin_job(GVJ_t * job)
{
    char *s;
    GVPUTS(job,
	   "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    if ((s = agget(job->gvc->g, "stylesheet")) && s[0]) {
	GVPUTS(job, "<?xml-stylesheet href=\"");
	gvputs(job, s);
	GVPUTS(job, "\" type=\"text/css\"?>\n");
    }
    GVPUTS(job, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
                " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"

                "<!-- Generated by ");
    gvputs_xml(job, job->common->info[0]);
    GVPUTS(job, " version ");
    gvputs_xml(job, job->common->info[1]);
    GVPUTS(job, " (");
    gvputs_xml(job, job->common->info[2]);
    GVPUTS(job, ")\n"
                " -->\n");
}

static void svg_begin_graph(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    GVPUTS(job, "<!--");
    if (agnameof(obj->u.g)[0] && agnameof(obj->u.g)[0] != LOCALNAMEPREFIX) {
	GVPUTS(job, " Title: ");
	gvputs_xml(job, agnameof(obj->u.g));
    }
    gvprintf(job, " Pages: %d -->\n",
	     job->pagesArraySize.x * job->pagesArraySize.y);

    gvprintf(job, "<svg width=\"%dpt\" height=\"%dpt\"\n",
	     job->width, job->height);
    gvprintf(job, " viewBox=\"%.2f %.2f %.2f %.2f\"",
	job->canvasBox.LL.x,
	job->canvasBox.LL.y,
	job->canvasBox.UR.x,
	job->canvasBox.UR.y);
    /* namespace of svg */
    GVPUTS(job, " xmlns=\"http://www.w3.org/2000/svg\""
    /* namespace of xlink */
                " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
                ">\n");
}

static void svg_end_graph(GVJ_t * job)
{
    GVPUTS(job, "</svg>\n");
}

static void svg_begin_layer(GVJ_t * job, char *layername, int layerNum,
			    int numLayers)
{
    (void)layerNum;
    (void)numLayers;

    obj_state_t *obj = job->obj;

    svg_print_id_class(job, layername, NULL, "layer", obj->u.g);
    GVPUTS(job, ">\n");
}

static void svg_end_layer(GVJ_t * job)
{
    GVPUTS(job, "</g>\n");
}

/* svg_begin_page:
 * Currently, svg output does not support pages.
 * FIX: If implemented, we must guarantee the id is unique.
 */
static void svg_begin_page(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    /* its really just a page of the graph, but its still a graph,
     * and it is the entire graph if we're not currently paging */
    svg_print_id_class(job, obj->id, NULL, "graph", obj->u.g);
    GVPUTS(job, " transform=\"scale(");
    gvprintdouble(job, job->scale.x);
    GVPUTS(job, " ");
    gvprintdouble(job, job->scale.y);
    gvprintf(job, ") rotate(%d) translate(", -job->rotation);
    gvprintdouble(job, job->translation.x);
    GVPUTS(job, " ");
    gvprintdouble(job, -job->translation.y);
    GVPUTS(job, ")\">\n");
    /* default style */
    if (agnameof(obj->u.g)[0] && agnameof(obj->u.g)[0] != LOCALNAMEPREFIX) {
	GVPUTS(job, "<title>");
	gvputs_xml(job, agnameof(obj->u.g));
	GVPUTS(job, "</title>\n");
    }
}

static void svg_end_page(GVJ_t * job)
{
    GVPUTS(job, "</g>\n");
}

static void svg_begin_cluster(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    svg_print_id_class(job, obj->id, NULL, "cluster", obj->u.sg);
    GVPUTS(job, ">\n"
                "<title>");
    gvputs_xml(job, agnameof(obj->u.g));
    GVPUTS(job, "</title>\n");
}

static void svg_end_cluster(GVJ_t * job)
{
    GVPUTS(job, "</g>\n");
}

static void svg_begin_node(GVJ_t * job)
{
    obj_state_t *obj = job->obj;
    char* idx;

    if (job->layerNum > 1)
	idx = job->gvc->layerIDs[job->layerNum];
    else
	idx = NULL;
    svg_print_id_class(job, obj->id, idx, "node", obj->u.n);
    GVPUTS(job, ">\n"
                "<title>");
    gvputs_xml(job, agnameof(obj->u.n));
    GVPUTS(job, "</title>\n");
}

static void svg_end_node(GVJ_t * job)
{
    GVPUTS(job, "</g>\n");
}

static void svg_begin_edge(GVJ_t * job)
{
    obj_state_t *obj = job->obj;
    char *ename;

    svg_print_id_class(job, obj->id, NULL, "edge", obj->u.e);
    GVPUTS(job, ">\n"

                "<title>");
    ename = strdup_and_subst_obj("\\E", obj->u.e);
    gvputs_xml(job, ename);
    free(ename);
    GVPUTS(job, "</title>\n");
}

static void svg_end_edge(GVJ_t * job)
{
    GVPUTS(job, "</g>\n");
}

static void
svg_begin_anchor(GVJ_t * job, char *href, char *tooltip, char *target,
		 char *id)
{
    GVPUTS(job, "<g");
    if (id) {
	GVPUTS(job, " id=\"a_");
        gvputs_xml(job, id);
        GVPUTS(job, "\"");
    }
    GVPUTS(job, ">"

                "<a");
    if (href && href[0]) {
	GVPUTS(job, " xlink:href=\"");
	const xml_flags_t flags = {0};
	xml_escape(href, flags, (int(*)(void*, const char*))gvputs, job);
	GVPUTS(job, "\"");
    }
    if (tooltip && tooltip[0]) {
	GVPUTS(job, " xlink:title=\"");
	const xml_flags_t flags = {.raw = 1, .dash = 1, .nbsp = 1};
	xml_escape(tooltip, flags, (int(*)(void*, const char*))gvputs, job);
	GVPUTS(job, "\"");
    }
    if (target && target[0]) {
	GVPUTS(job, " target=\"");
	gvputs_xml(job, target);
	GVPUTS(job, "\"");
    }
    GVPUTS(job, ">\n");
}

static void svg_end_anchor(GVJ_t * job)
{
    GVPUTS(job, "</a>\n"
                "</g>\n");
}

static void svg_textspan(GVJ_t * job, pointf p, textspan_t * span)
{
    obj_state_t *obj = job->obj;
    PostscriptAlias *pA;
    char *family = NULL, *weight = NULL, *stretch = NULL, *style = NULL;
    unsigned int flags;

    GVPUTS(job, "<text");
    switch (span->just) {
    case 'l':
	GVPUTS(job, " text-anchor=\"start\"");
	break;
    case 'r':
	GVPUTS(job, " text-anchor=\"end\"");
	break;
    default:
    case 'n':
	GVPUTS(job, " text-anchor=\"middle\"");
	break;
    }
    p.y += span->yoffset_centerline;
    if (!obj->labeledgealigned) {
	GVPUTS(job, " x=\"");
        gvprintdouble(job, p.x);
        GVPUTS(job, "\" y=\"");
        gvprintdouble(job, -p.y);
        GVPUTS(job, "\"");
    }
    pA = span->font->postscript_alias;
    if (pA) {
	switch (GD_fontnames(job->gvc->g)) {
	case PSFONTS:
	    family = pA->name;
	    weight = pA->weight;
	    style = pA->style;
	    break;
	case SVGFONTS:
	    family = pA->svg_font_family;
	    weight = pA->svg_font_weight;
	    style = pA->svg_font_style;
	    break;
	default:
	case NATIVEFONTS:
	    family = pA->family;
	    weight = pA->weight;
	    style = pA->style;
	    break;
	}
	stretch = pA->stretch;

	gvprintf(job, " font-family=\"%s", family);
	if (pA->svg_font_family)
	    gvprintf(job, ",%s", pA->svg_font_family);
	GVPUTS(job, "\"");
	if (weight)
	    gvprintf(job, " font-weight=\"%s\"", weight);
	if (stretch)
	    gvprintf(job, " font-stretch=\"%s\"", stretch);
	if (style)
	    gvprintf(job, " font-style=\"%s\"", style);
    } else
	gvprintf(job, " font-family=\"%s\"", span->font->name);
    if ((span->font) && (flags = span->font->flags)) {
	if ((flags & HTML_BF) && !weight)
	    GVPUTS(job, " font-weight=\"bold\"");
	if ((flags & HTML_IF) && !style)
	    GVPUTS(job, " font-style=\"italic\"");
	if ((flags & (HTML_UL|HTML_S|HTML_OL))) {
	    int comma = 0;
	    GVPUTS(job, " text-decoration=\"");
	    if ((flags & HTML_UL)) {
		GVPUTS(job, "underline");
		comma = 1;
	    }
	    if ((flags & HTML_OL)) {
		gvprintf(job, "%soverline", (comma?",":""));
		comma = 1;
	    }
	    if ((flags & HTML_S))
		gvprintf(job, "%sline-through", (comma?",":""));
	    GVPUTS(job, "\"");
	}
	if ((flags & HTML_SUP))
	    GVPUTS(job, " baseline-shift=\"super\"");
	if ((flags & HTML_SUB))
	    GVPUTS(job, " baseline-shift=\"sub\"");
    }

    gvprintf(job, " font-size=\"%.2f\"", span->font->size);
    switch (obj->pencolor.type) {
    case COLOR_STRING:
	if (strcasecmp(obj->pencolor.u.string, "black"))
	    gvprintf(job, " fill=\"%s\"", obj->pencolor.u.string);
	break;
    case RGBA_BYTE:
	gvprintf(job, " fill=\"#%02x%02x%02x\"",
		 obj->pencolor.u.rgba[0], obj->pencolor.u.rgba[1],
		 obj->pencolor.u.rgba[2]);
	if (obj->pencolor.u.rgba[3] > 0 && obj->pencolor.u.rgba[3] < 255)
	    gvprintf(job, " fill-opacity=\"%f\"", ((float) obj->pencolor.u.rgba[3] / 255.0));
	break;
    default:
	assert(0);		/* internal error */
    }
    GVPUTS(job, ">");
    if (obj->labeledgealigned) {
	GVPUTS(job, "<textPath xlink:href=\"#");
	gvputs_xml(job, obj->id);
	GVPUTS(job, "_p\" startOffset=\"50%\"><tspan x=\"0\" dy=\"");
        gvprintdouble(job, -p.y);
        GVPUTS(job, "\">");
    }
    const xml_flags_t xml_flags = {.raw = 1, .dash = 1, .nbsp = 1};
    xml_escape(span->str, xml_flags, (int(*)(void*, const char*))gvputs, job);
    if (obj->labeledgealigned)
	GVPUTS(job, "</tspan></textPath>");
    GVPUTS(job, "</text>\n");
}

/* svg_gradstyle
 * Outputs the SVG statements that define the gradient pattern
 */
static int svg_gradstyle(GVJ_t * job, pointf * A, int n)
{
    pointf G[2];
    float angle;
    static int gradId;
    int id = gradId++;

    obj_state_t *obj = job->obj;
    angle = obj->gradient_angle * M_PI / 180;	//angle of gradient line
    G[0].x = G[0].y = G[1].x = G[1].y = 0.;
    get_gradient_points(A, G, n, angle, 0);	//get points on gradient line

    gvprintf(job,
	     "<defs>\n<linearGradient id=\"l_%d\" gradientUnits=\"userSpaceOnUse\" ", id);
    GVPUTS(job, "x1=\"");
    gvprintdouble(job, G[0].x);
    GVPUTS(job, "\" y1=\"");
    gvprintdouble(job, G[0].y);
    GVPUTS(job, "\" x2=\"");
    gvprintdouble(job, G[1].x);
    GVPUTS(job, "\" y2=\"");
    gvprintdouble(job, G[1].y);
    GVPUTS(job, "\" >\n");
    if (obj->gradient_frac > 0)
	gvprintf(job, "<stop offset=\"%.03f\" style=\"stop-color:", obj->gradient_frac - 0.001);
    else
	GVPUTS(job, "<stop offset=\"0\" style=\"stop-color:");
    svg_print_color(job, obj->fillcolor);
    GVPUTS(job, ";stop-opacity:");
    if (obj->fillcolor.type == RGBA_BYTE && obj->fillcolor.u.rgba[3] > 0
	&& obj->fillcolor.u.rgba[3] < 255)
	gvprintf(job, "%f", ((float) obj->fillcolor.u.rgba[3] / 255.0));
    else
	GVPUTS(job, "1.");
    GVPUTS(job, ";\"/>\n");
    if (obj->gradient_frac > 0)
	gvprintf(job, "<stop offset=\"%.03f\" style=\"stop-color:", obj->gradient_frac);
    else
	GVPUTS(job, "<stop offset=\"1\" style=\"stop-color:");
    svg_print_color(job, obj->stopcolor);
    GVPUTS(job, ";stop-opacity:");
    if (obj->stopcolor.type == RGBA_BYTE && obj->stopcolor.u.rgba[3] > 0
	&& obj->stopcolor.u.rgba[3] < 255)
	gvprintf(job, "%f", ((float) obj->stopcolor.u.rgba[3] / 255.0));
    else
	GVPUTS(job, "1.");
    GVPUTS(job, ";\"/>\n</linearGradient>\n</defs>\n");
    return id;
}

/* svg_rgradstyle
 * Outputs the SVG statements that define the radial gradient pattern
 */
static int svg_rgradstyle(GVJ_t * job)
{
    float angle;
    int ifx, ify;
    static int rgradId;
    int id = rgradId++;

    obj_state_t *obj = job->obj;
    angle = obj->gradient_angle * M_PI / 180;	//angle of gradient line
    if (angle == 0.) {
	ifx = ify = 50;
    } else {
	ifx = 50 * (1 + cos(angle));
	ify = 50 * (1 - sin(angle));
    }
    gvprintf(job,
	     "<defs>\n<radialGradient id=\"r_%d\" cx=\"50%%\" cy=\"50%%\" r=\"75%%\" fx=\"%d%%\" fy=\"%d%%\">\n",
	     id, ifx, ify);
    GVPUTS(job, "<stop offset=\"0\" style=\"stop-color:");
    svg_print_color(job, obj->fillcolor);
    GVPUTS(job, ";stop-opacity:");
    if (obj->fillcolor.type == RGBA_BYTE && obj->fillcolor.u.rgba[3] > 0
	&& obj->fillcolor.u.rgba[3] < 255)
	gvprintf(job, "%f", ((float) obj->fillcolor.u.rgba[3] / 255.0));
    else
	GVPUTS(job, "1.");
    GVPUTS(job, ";\"/>\n"
                "<stop offset=\"1\" style=\"stop-color:");
    svg_print_color(job, obj->stopcolor);
    GVPUTS(job, ";stop-opacity:");
    if (obj->stopcolor.type == RGBA_BYTE && obj->stopcolor.u.rgba[3] > 0
	&& obj->stopcolor.u.rgba[3] < 255)
	gvprintf(job, "%f", ((float) obj->stopcolor.u.rgba[3] / 255.0));
    else
	GVPUTS(job, "1.");
    GVPUTS(job, ";\"/>\n</radialGradient>\n</defs>\n");
    return id;
}


static void svg_ellipse(GVJ_t * job, pointf * A, int filled)
{
    int gid = 0;

    /* A[] contains 2 points: the center and corner. */
    if (filled == GRADIENT) {
	gid = svg_gradstyle(job, A, 2);
    } else if (filled == (RGRADIENT)) {
	gid = svg_rgradstyle(job);
    }
    GVPUTS(job, "<ellipse");
    svg_grstyle(job, filled, gid);
    GVPUTS(job, " cx=\"");
    gvprintdouble(job, A[0].x);
    GVPUTS(job, "\" cy=\"");
    gvprintdouble(job, -A[0].y);
    GVPUTS(job, "\" rx=\"");
    gvprintdouble(job, A[1].x - A[0].x);
    GVPUTS(job, "\" ry=\"");
    gvprintdouble(job, A[1].y - A[0].y);
    GVPUTS(job, "\"/>\n");
}

static void
svg_bezier(GVJ_t * job, pointf * A, int n, int arrow_at_start,
	   int arrow_at_end, int filled)
{
    (void)arrow_at_start;
    (void)arrow_at_end;

    int gid = 0;
    obj_state_t *obj = job->obj;
  
    if (filled == GRADIENT) {
	gid = svg_gradstyle(job, A, n);
    } else if (filled == (RGRADIENT)) {
	gid = svg_rgradstyle(job);
    }
    GVPUTS(job, "<path");
    if (obj->labeledgealigned) {
	GVPUTS(job, " id=\"");
	gvputs_xml(job, obj->id);
	GVPUTS(job, "_p\" ");
    } 
    svg_grstyle(job, filled, gid);
    GVPUTS(job, " d=\"");
    svg_bzptarray(job, A, n);
    GVPUTS(job, "\"/>\n");
}

static void svg_polygon(GVJ_t * job, pointf * A, int n, int filled)
{
    int i, gid = 0;
    if (filled == GRADIENT) {
	gid = svg_gradstyle(job, A, n);
    } else if (filled == (RGRADIENT)) {
	gid = svg_rgradstyle(job);
    }
    GVPUTS(job, "<polygon");
    svg_grstyle(job, filled, gid);
    GVPUTS(job, " points=\"");
    for (i = 0; i < n; i++) {
        gvprintdouble(job, A[i].x);
        GVPUTS(job, ",");
        gvprintdouble(job, -A[i].y);
        GVPUTS(job, " ");
    }
    /* repeat the first point because Adobe SVG is broken */
    gvprintdouble(job, A[0].x);
    GVPUTS(job, ",");
    gvprintdouble(job, -A[0].y);
    GVPUTS(job, "\"/>\n");
}

static void svg_polyline(GVJ_t * job, pointf * A, int n)
{
    int i;

    GVPUTS(job, "<polyline");
    svg_grstyle(job, 0, 0);
    GVPUTS(job, " points=\"");
    for (i = 0; i < n; i++) {
        gvprintdouble(job, A[i].x);
        GVPUTS(job, ",");
        gvprintdouble(job, -A[i].y);
        GVPUTS(job, " ");
    }
    GVPUTS(job, "\"/>\n");
}

/* color names from http://www.w3.org/TR/SVG/types.html */
/* NB.  List must be LANG_C sorted */
static char *svg_knowncolors[] = {
    "aliceblue", "antiquewhite", "aqua", "aquamarine", "azure",
    "beige", "bisque", "black", "blanchedalmond", "blue",
    "blueviolet", "brown", "burlywood",
    "cadetblue", "chartreuse", "chocolate", "coral",
    "cornflowerblue", "cornsilk", "crimson", "cyan",
    "darkblue", "darkcyan", "darkgoldenrod", "darkgray",
    "darkgreen", "darkgrey", "darkkhaki", "darkmagenta",
    "darkolivegreen", "darkorange", "darkorchid", "darkred",
    "darksalmon", "darkseagreen", "darkslateblue", "darkslategray",
    "darkslategrey", "darkturquoise", "darkviolet", "deeppink",
    "deepskyblue", "dimgray", "dimgrey", "dodgerblue",
    "firebrick", "floralwhite", "forestgreen", "fuchsia",
    "gainsboro", "ghostwhite", "gold", "goldenrod", "gray",
    "green", "greenyellow", "grey",
    "honeydew", "hotpink", "indianred",
    "indigo", "ivory", "khaki",
    "lavender", "lavenderblush", "lawngreen", "lemonchiffon",
    "lightblue", "lightcoral", "lightcyan", "lightgoldenrodyellow",
    "lightgray", "lightgreen", "lightgrey", "lightpink",
    "lightsalmon", "lightseagreen", "lightskyblue",
    "lightslategray", "lightslategrey", "lightsteelblue",
    "lightyellow", "lime", "limegreen", "linen",
    "magenta", "maroon", "mediumaquamarine", "mediumblue",
    "mediumorchid", "mediumpurple", "mediumseagreen",
    "mediumslateblue", "mediumspringgreen", "mediumturquoise",
    "mediumvioletred", "midnightblue", "mintcream",
    "mistyrose", "moccasin",
    "navajowhite", "navy", "oldlace",
    "olive", "olivedrab", "orange", "orangered", "orchid",
    "palegoldenrod", "palegreen", "paleturquoise",
    "palevioletred", "papayawhip", "peachpuff", "peru", "pink",
    "plum", "powderblue", "purple",
    "red", "rosybrown", "royalblue",
    "saddlebrown", "salmon", "sandybrown", "seagreen", "seashell",
    "sienna", "silver", "skyblue", "slateblue", "slategray",
    "slategrey", "snow", "springgreen", "steelblue",
    "tan", "teal", "thistle", "tomato", "transparent", "turquoise",
    "violet",
    "wheat", "white", "whitesmoke",
    "yellow", "yellowgreen"
};

gvrender_engine_t svg_engine = {
    svg_begin_job,
    0,				/* svg_end_job */
    svg_begin_graph,
    svg_end_graph,
    svg_begin_layer,
    svg_end_layer,
    svg_begin_page,
    svg_end_page,
    svg_begin_cluster,
    svg_end_cluster,
    0,				/* svg_begin_nodes */
    0,				/* svg_end_nodes */
    0,				/* svg_begin_edges */
    0,				/* svg_end_edges */
    svg_begin_node,
    svg_end_node,
    svg_begin_edge,
    svg_end_edge,
    svg_begin_anchor,
    svg_end_anchor,
    0,				/* svg_begin_anchor */
    0,				/* svg_end_anchor */
    svg_textspan,
    0,				/* svg_resolve_color */
    svg_ellipse,
    svg_polygon,
    svg_bezier,
    svg_polyline,
    svg_comment,
    0,				/* svg_library_shape */
};

gvrender_features_t render_features_svg = {
    GVRENDER_Y_GOES_DOWN | GVRENDER_DOES_TRANSFORM | GVRENDER_DOES_LABELS | GVRENDER_DOES_MAPS | GVRENDER_DOES_TARGETS | GVRENDER_DOES_TOOLTIPS,	/* flags */
    4.,				/* default pad - graph units */
    svg_knowncolors,		/* knowncolors */
    sizeof(svg_knowncolors) / sizeof(char *),	/* sizeof knowncolors */
    RGBA_BYTE,			/* color_type */
};

gvdevice_features_t device_features_svg = {
    GVDEVICE_DOES_TRUECOLOR|GVDEVICE_DOES_LAYERS,  /* flags */
    {0., 0.},			/* default margin - points */
    {0., 0.},			/* default page width, height - points */
    {72., 72.},			/* default dpi */
};

gvdevice_features_t device_features_svgz = {
    GVDEVICE_DOES_TRUECOLOR|GVDEVICE_DOES_LAYERS|GVDEVICE_BINARY_FORMAT|GVDEVICE_COMPRESSED_FORMAT, /* flags */
    {0., 0.},			/* default margin - points */
    {0., 0.},			/* default page width, height - points */
    {72., 72.},			/* default dpi */
};

gvplugin_installed_t gvrender_svg_types[] = {
    {FORMAT_SVG, "svg", 1, &svg_engine, &render_features_svg},
    {0, NULL, 0, NULL, NULL}
};

gvplugin_installed_t gvdevice_svg_types[] = {
    {FORMAT_SVG, "svg:svg", 1, NULL, &device_features_svg},
#if HAVE_LIBZ
    {FORMAT_SVGZ, "svgz:svg", 1, NULL, &device_features_svgz},
#endif
    {0, NULL, 0, NULL, NULL}
};
