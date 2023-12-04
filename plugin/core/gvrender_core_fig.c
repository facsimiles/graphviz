/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#endif

#include <common/macros.h>
#include <common/const.h>

#include <gvc/gvplugin_render.h>
#include <gvc/gvplugin_device.h>
#include <gvc/gvio.h>
#include <cgraph/agxbuf.h>
#include <cgraph/unreachable.h>
#include <common/utils.h>
#include <common/color.h>

/* Number of points to split splines into */
#define BEZIERSUBDIVISION 6

typedef enum { FORMAT_FIG, } format_type;

static int Depth;

static void figptarray(GVJ_t *job, pointf * A, int n, int close)
{
    int i;
    point p;

    for (i = 0; i < n; i++) {
	PF2P(A[i],p);
        gvprintf(job, " %d %d", p.x, p.y);
    }
    if (close) {
	PF2P(A[0],p);
        gvprintf(job, " %d %d", p.x, p.y);
    }
    gvputs(job, "\n");
}

static int figColorResolve(int *new, unsigned char r, unsigned char g,
  unsigned char b)
{
#define maxColors 256
    static int top = 0;
    static short red[maxColors], green[maxColors], blue[maxColors];
    int c;
    int ct = -1;
    long rd, gd, bd, dist;
    long mindist = 3 * 255 * 255;       /* init to max poss dist */

    *new = 0;                   /* in case it is not a new color */
    for (c = 0; c < top; c++) {
        rd = (long) (red[c] - r);
        gd = (long) (green[c] - g);
        bd = (long) (blue[c] - b);
        dist = rd * rd + gd * gd + bd * bd;
        if (dist < mindist) {
            if (dist == 0)
                return c;       /* Return exact match color */
            mindist = dist;
            ct = c;
        }
    }
    /* no exact match.  We now know closest, but first try to allocate exact */
    if (top++ == maxColors)
        return ct;              /* Return closest available color */
    red[c] = r;
    green[c] = g;
    blue[c] = b;
    *new = 1;                   /* flag new color */
    return c;                   /* Return newly allocated color */
}

/* this table is in xfig color index order */
static char *figcolor[] = {
    "black", "blue", "green", "cyan", "red", "magenta", "yellow", "white", (char *) NULL
};

static void fig_resolve_color(GVJ_t *job, gvcolor_t * color)
{
    int object_code = 0;        /* always 0 for color */
    int i, new;

    switch (color->type) {
	case COLOR_STRING:
	    for (i = 0; figcolor[i]; i++) {
		if (streq(figcolor[i], color->u.string)) {
		    color->u.index = i;
		    break;
		}
	    }
	    break;
	case RGBA_BYTE:
	    i = 32 + figColorResolve(&new,
			color->u.rgba[0],
			color->u.rgba[1],
			color->u.rgba[2]);
	    if (new)
		gvprintf(job, "%d %d #%02x%02x%02x\n",
			object_code, i,
			color->u.rgba[0],
			color->u.rgba[1],
			color->u.rgba[2]);
	    color->u.index = i;
	    break;
	default:
	    UNREACHABLE(); // internal error
    }

    color->type = COLOR_INDEX;
}

static void fig_line_style(obj_state_t *obj, int *line_style, double *style_val)
{
    switch (obj->pen) {
	case PEN_DASHED: 
	    *line_style = 1;
	    *style_val = 10.;
	    break;
	case PEN_DOTTED:
	    *line_style = 2;
	    *style_val = 10.;
	    break;
	case PEN_SOLID:
	default:
	    *line_style = 0;
	    *style_val = 0.;
	    break;
    }
}

static void fig_comment(GVJ_t *job, char *str)
{
    gvprintf(job, "# %s\n", str);
}

static void fig_begin_graph(GVJ_t * job)
{
    obj_state_t *obj = job->obj;

    gvputs(job, "#FIG 3.2\n");
    gvprintf(job, "# Generated by %s version %s (%s)\n",
	job->common->info[0], job->common->info[1], job->common->info[2]);
    gvprintf(job, "# Title: %s\n", agnameof(obj->u.g));
    gvprintf(job, "# Pages: %d\n", job->pagesArraySize.x * job->pagesArraySize.y);
    gvputs(job, "Portrait\n"); /* orientation */
    gvputs(job, "Center\n");   /* justification */
    gvputs(job, "Inches\n");   /* units */
    gvputs(job, "Letter\n");   /* papersize */
    gvputs(job, "100.00\n");   /* magnification % */
    gvputs(job, "Single\n");   /* multiple-page */
    gvputs(job, "-2\n");       /* transparent color (none) */
    gvputs(job, "1200");	     /* resolution */
    gvputs(job, " 2\n");       /* coordinate system (upper left) */
}

static void fig_end_graph(GVJ_t * job)
{
    gvputs(job, "# end of FIG file\n");
}

static void fig_begin_page(GVJ_t * job)
{
    (void)job;

    Depth = 2;
}

static void fig_begin_node(GVJ_t * job)
{
    (void)job;

    Depth = 1;
}

static void fig_end_node(GVJ_t * job)
{
    (void)job;

    Depth = 2;
}

static void fig_begin_edge(GVJ_t * job)
{
    (void)job;

    Depth = 0;
}

static void fig_end_edge(GVJ_t * job)
{
    (void)job;

    Depth = 2;
}

static void fig_textspan(GVJ_t * job, pointf p, textspan_t * span)
{
    obj_state_t *obj = job->obj;
    PostscriptAlias *pA;

    int object_code = 4;        /* always 4 for text */
    int sub_type = 0;           /* text justification */
    int color = obj->pencolor.u.index;
    int depth = Depth;
    int pen_style = 0;          /* not used */
    int font = -1;		/* init to xfig's default font */
    double font_size = span->font->size * job->zoom;
    double angle = job->rotation ? (M_PI / 2.0) : 0.0;
    int font_flags = 6;		/* PostScript font + Special text */
/* Special text indicates that latex markup may exist
 * in the output - but note that dot knows nothing about latex,
 * so the node sizes may be wrong.
 */
    double height = font_size;
    double length = 2.0*font_size/3.0  * (double)strlen(span->str) / 2.0;

    pA = span->font->postscript_alias;
    if (pA) /* if it is a standard postscript font */
	font = pA->xfig_code; 

    switch (span->just) {
    case 'l':
        sub_type = 0;
        break;
    case 'r':
        sub_type = 2;
        break;
    default:
    case 'n':
        sub_type = 1;
        break;
    }

/*	object_code	sub_type	color	depth	pen_style	font
	4			1		0		1		0		0	
         font_size	angle	font_flags	height	length	ROUND(p.x)	ROUND(p.y),
	14.0		0.0000	6		14.0		51.3		1237		570	
	$A	\\in	M_0$\001
*/
    gvprintf(job,
            "%d %d %d %d %d %d %.1f %.4f %d %.1f %.1f %d %d ",
            object_code, sub_type, color, depth, pen_style, font,
            font_size, angle, font_flags, height, length, ROUND(p.x), ROUND((p.y-72.0)));
    gvputs_nonascii(job, span->str);
    gvputs(job, "\\001\n");
}

static void fig_ellipse(GVJ_t * job, pointf * A, int filled)
{
    obj_state_t *obj = job->obj;

    int object_code = 1;        /* always 1 for ellipse */
    int sub_type = 1;           /* ellipse defined by radii */
    int line_style;		/* solid, dotted, dashed */
    double thickness = round(obj->penwidth);
    int pen_color = obj->pencolor.u.index;
    int fill_color = obj->fillcolor.u.index;
    int depth = Depth;
    int pen_style = 0;          /* not used */
    int area_fill = filled ? 20 : -1;
    double style_val;
    int direction = 0;
    double angle = 0.0;
    int center_x, center_y, radius_x, radius_y;
    int start_x, start_y, end_x, end_y;

    fig_line_style(obj, &line_style, &style_val);

    start_x = center_x = ROUND(A[0].x);
    start_y = center_y = ROUND(A[0].y);
    radius_x = ROUND(A[1].x - A[0].x);
    radius_y = ROUND(A[1].y - A[0].y);
    end_x = ROUND(A[1].x);
    end_y = ROUND(A[1].y);

    gvprintf(job,
            "%d %d %d %.0f %d %d %d %d %d %.3f %d %.4f %d %d %d %d %d %d %d %d\n",
            object_code, sub_type, line_style, thickness, pen_color,
            fill_color, depth, pen_style, area_fill, style_val, direction,
            angle, center_x, center_y, radius_x, radius_y, start_x,
            start_y, end_x, end_y);
}

static void fig_bezier(GVJ_t *job, pointf *A, int n, int filled) {
    obj_state_t *obj = job->obj;

    int object_code = 3;        /* always 3 for spline */
    int sub_type;
    int line_style;		/* solid, dotted, dashed */
    double thickness = round(obj->penwidth);
    int pen_color = obj->pencolor.u.index;
    int fill_color = obj->fillcolor.u.index;
    int depth = Depth;
    int pen_style = 0;          /* not used */
    int area_fill;
    double style_val;
    int cap_style = 0;
    int forward_arrow = 0;
    int backward_arrow = 0;
    int i;


    pointf pf, V[4];
    point p;
    int j, step;
    int count = 0;

    agxbuf buf = {0};
    assert (n >= 4);

    fig_line_style(obj, &line_style, &style_val);

    if (filled) {
        sub_type = 5;     /* closed X-spline */
        area_fill = 20;   /* fully saturated color */
        fill_color = job->obj->fillcolor.u.index;
    }
    else {
        sub_type = 4;     /* opened X-spline */
        area_fill = -1;
        fill_color = 0;
    }
    V[3].x = A[0].x;
    V[3].y = A[0].y;
    /* Write first point in line */
    count++;
    PF2P(A[0], p);
    agxbprint(&buf, " %d %d", p.x, p.y);
    /* write subsequent points */
    for (i = 0; i + 3 < n; i += 3) {
        V[0] = V[3];
        for (j = 1; j <= 3; j++) {
            V[j].x = A[i + j].x;
            V[j].y = A[i + j].y;
        }
        for (step = 1; step <= BEZIERSUBDIVISION; step++) {
            count++;
            pf = Bezier(V, (double)step / BEZIERSUBDIVISION, NULL, NULL);
	    PF2P(pf, p);
            agxbprint(&buf, " %d %d", p.x, p.y);
        }
    }

    gvprintf(job, "%d %d %d %.0f %d %d %d %d %d %.1f %d %d %d %d\n",
            object_code,
            sub_type,
            line_style,
            thickness,
            pen_color,
            fill_color,
            depth,
            pen_style,
            area_fill,
            style_val, cap_style, forward_arrow, backward_arrow, count);

    gvprintf(job, " %s\n", agxbuse(&buf));      /* print points */
    agxbfree(&buf);
    for (i = 0; i < count; i++) {
        gvprintf(job, " %d", i % (count - 1) ? 1 : 0);   /* -1 on all */
    }
    gvputs(job, "\n");
}

static void fig_polygon(GVJ_t * job, pointf * A, int n, int filled)
{
    obj_state_t *obj = job->obj;

    int object_code = 2;        /* always 2 for polyline */
    int sub_type = 3;           /* always 3 for polygon */
    int line_style;		/* solid, dotted, dashed */
    double thickness = round(obj->penwidth);
    int pen_color = obj->pencolor.u.index;
    int fill_color = obj->fillcolor.u.index;
    int depth = Depth;
    int pen_style = 0;          /* not used */
    int area_fill = filled ? 20 : -1;
    double style_val;
    int join_style = 0;
    int cap_style = 0;
    int radius = 0;
    int forward_arrow = 0;
    int backward_arrow = 0;
    int npoints = n + 1;

    fig_line_style(obj, &line_style, &style_val);

    gvprintf(job,
            "%d %d %d %.0f %d %d %d %d %d %.1f %d %d %d %d %d %d\n",
            object_code, sub_type, line_style, thickness, pen_color,
            fill_color, depth, pen_style, area_fill, style_val, join_style,
            cap_style, radius, forward_arrow, backward_arrow, npoints);
    figptarray(job, A, n, 1);        /* closed shape */
}

static void fig_polyline(GVJ_t * job, pointf * A, int n)
{
    obj_state_t *obj = job->obj;

    int object_code = 2;        /* always 2 for polyline */
    int sub_type = 1;           /* always 1 for polyline */
    int line_style;		/* solid, dotted, dashed */
    double thickness = round(obj->penwidth);
    int pen_color = obj->pencolor.u.index;
    int fill_color = 0;
    int depth = Depth;
    int pen_style = 0;          /* not used */
    int area_fill = 0;
    double style_val;
    int join_style = 0;
    int cap_style = 0;
    int radius = 0;
    int forward_arrow = 0;
    int backward_arrow = 0;
    int npoints = n;

    fig_line_style(obj, &line_style, &style_val);

    gvprintf(job,
            "%d %d %d %.0f %d %d %d %d %d %.1f %d %d %d %d %d %d\n",
            object_code, sub_type, line_style, thickness, pen_color,
            fill_color, depth, pen_style, area_fill, style_val, join_style,
            cap_style, radius, forward_arrow, backward_arrow, npoints);
    figptarray(job, A, n, 0);        /* open shape */
}

gvrender_engine_t fig_engine = {
    0,				/* fig_begin_job */
    0,				/* fig_end_job */
    fig_begin_graph,
    fig_end_graph,
    0,				/* fig_begin_layer */
    0,				/* fig_end_layer */
    fig_begin_page,
    0,				/* fig_end_page */
    0,				/* fig_begin_cluster */
    0,				/* fig_end_cluster */
    0,				/* fig_begin_nodes */
    0,				/* fig_end_nodes */
    0,				/* fig_begin_edges */
    0,				/* fig_end_edges */
    fig_begin_node,
    fig_end_node,
    fig_begin_edge,
    fig_end_edge,
    0,				/* fig_begin_anchor */
    0,				/* fig_end_anchor */
    0,				/* fig_begin_label */
    0,				/* fig_end_label */
    fig_textspan,
    fig_resolve_color,
    fig_ellipse,
    fig_polygon,
    fig_bezier,
    fig_polyline,
    fig_comment,
    0,				/* fig_library_shape */
};


/* NB.  List must be LANG_C sorted */
static char *fig_knowncolors[] = {
    "black", "blue", "cyan", "green", "magenta", "red", "white", "yellow",
};


gvrender_features_t render_features_fig = {
    EMIT_COLORS
	| GVRENDER_Y_GOES_DOWN,	/* flags */
    4.,                         /* default pad - graph units */
    fig_knowncolors,		/* knowncolors */
    sizeof(fig_knowncolors) / sizeof(char *), /* sizeof knowncolors */
    RGBA_BYTE,			/* color_type */
};

gvdevice_features_t device_features_fig = {
    EMIT_COLORS
	| GVRENDER_Y_GOES_DOWN,	/* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {1440.,1440.},		/* default dpi */
   	 /* FIXME - this default dpi is a very strange number!!!
	  * It was picked to make .png usershapes the right size on my screen.
	  * It happens to be 1.2 * 1200, but I can't explain the 1.2.
	  * (I was expecting 1.3333 which is 96/72, but thats too big.)
    	  * Also 1200 is hardcoded in fig_begin_graph() instead of using job->dpi 
          */

	 /* It may be TWIPS, i.e. 20 * POINT_PER_INCH 
	  *    but that doesn't explain what the 1200 is? */
};

gvplugin_installed_t gvrender_fig_types[] = {
    {FORMAT_FIG, "fig", 1, &fig_engine, &render_features_fig},
    {0, NULL, 0, NULL, NULL}
};

gvplugin_installed_t gvdevice_fig_types[] = {
    {FORMAT_FIG, "fig:fig", 1, NULL, &device_features_fig},
    {0, NULL, 0, NULL, NULL}
};
