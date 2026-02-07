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
#include "gdioctx_wrapper.h"
#include "gdgen_text.h"
#include "gd_psfontResolve.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <gvc/gvplugin_render.h>
#include <gvc/gvplugin_device.h>
#include <gvc/gvcint.h>	/* for gvc->g for agget */
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#include <util/alloc.h>
#include <util/gv_math.h>
#include <util/unreachable.h>

enum {
	FORMAT_GIF,
	FORMAT_JPEG,
	FORMAT_PNG,
	FORMAT_WBMP,
	FORMAT_GD,
	FORMAT_GD2,
	FORMAT_XBM,
};

extern bool mapbool(const char *);
extern pointf Bezier(pointf *V, double t, pointf *Left, pointf *Right);

#define BEZIERSUBDIVISION 10

static void gdgen_resolve_color(GVJ_t * job, gvcolor_t * color)
{
    gdImagePtr im = job->context;
    int alpha;

    if (!im)
	return;

    /* convert alpha (normally an "opacity" value) to gd's "transparency" */
    alpha = (255 - color->u.rgba[3]) * gdAlphaMax / 255;

    if(alpha == gdAlphaMax)
	color->u.index = gdImageGetTransparent(im);
    else
        color->u.index = gdImageColorResolveAlpha(im,
			  color->u.rgba[0],
			  color->u.rgba[1],
			  color->u.rgba[2],
			  alpha);
    color->type = COLOR_INDEX;
}

static int transparent, basecolor;

#define GD_XYMAX INT32_MAX

static void gdgen_begin_page(GVJ_t * job)
{
    bool truecolor_p = false;	/* try to use cheaper paletted mode */
    gdImagePtr im = NULL;

    const char *const truecolor_str = agget(job->gvc->g, "truecolor"); // allow user to force truecolor
    const char *const bgcolor_str = agget(job->gvc->g, "bgcolor");

    if (truecolor_str && truecolor_str[0])
	truecolor_p = mapbool(truecolor_str);

    if (bgcolor_str && strcmp(bgcolor_str, "transparent") == 0) {
	if (job->render.features->flags & GVDEVICE_DOES_TRUECOLOR)
	    truecolor_p = true;	/* force truecolor */
    }

    if (GD_has_images(job->gvc->g))
	truecolor_p = true;	/* force truecolor */

    if (job->external_context) {
	if (job->common->verbose)
	    fprintf(stderr, "%s: using existing GD image\n", job->common->cmdname);
	im = job->context;
    } else {
        if (job->width * job->height >= GD_XYMAX) {
	    double scale = sqrt(GD_XYMAX / (job->width * job->height));
	    assert(scale > 0 && scale <= 1);
	    job->width = (unsigned)(job->width * scale);
	    job->height = (unsigned)(job->height * scale);
	    job->zoom *= scale;
	    fprintf(stderr,
		"%s: graph is too large for gd-renderer bitmaps. Scaling by %g to fit\n",
		job->common->cmdname, scale);
	}
	assert(job->width <= INT_MAX);
	assert(job->height <= INT_MAX);
	if (truecolor_p) {
	    if (job->common->verbose)
		fprintf(stderr,
			"%s: allocating a %0.fK TrueColor GD image (%d x %d pixels)\n",
			job->common->cmdname,
			round(job->width * job->height * 4 / 1024.),
			job->width, job->height);
	    im = gdImageCreateTrueColor((int)job->width, (int)job->height);
	} else {
	    if (job->common->verbose)
		fprintf(stderr,
			"%s: allocating a %.0fK PaletteColor GD image (%d x %d pixels)\n",
			job->common->cmdname,
			round(job->width * job->height / 1024.),
			job->width, job->height);
	    im = gdImageCreate((int)job->width, (int)job->height);
	}
        job->context = im;
    }

    if (!im) {
	job->common->errorfn("gdImageCreate returned NULL. Malloc problem?\n");
	return;
    }

    /* first color is the default background color */
    /*   - used for margins - if any */
    transparent = gdImageColorResolveAlpha(im,
					   gdRedMax - 1, gdGreenMax,
					   gdBlueMax, gdAlphaTransparent);
    gdImageColorTransparent(im, transparent);

    /* Blending must be off to lay a transparent basecolor.
       Nothing to blend with anyway. */
    gdImageAlphaBlending(im, false);
    gdImageFill(im, im->sx / 2, im->sy / 2, transparent);
    /* Blend everything else together,
       especially fonts over non-transparent backgrounds */
    gdImageAlphaBlending(im, true);
}

static void gdgen_end_page(GVJ_t * job)
{
    gdImagePtr im = job->context;

    gd_context_t gd_context = {
	.ctx = {.putBuf = gvdevice_gd_putBuf, .putC = gvdevice_gd_putC},
	.job = job,
    };

    if (!im)
	return;
    if (job->external_context) {
	/* leave image in memory to be handled by Gdtclft output routines */
    } else {
	/* Only save the alpha channel in outputs that support it if
	   the base color was transparent.   Otherwise everything
	   was blended so there is no useful alpha info */
	gdImageSaveAlpha(im, basecolor == transparent);
	switch (job->render.id) {
	case FORMAT_GIF:
	    gdImageTrueColorToPalette(im, 0, 256);
	    gdImageGifCtx(im, &gd_context.ctx);
	    break;
	case FORMAT_JPEG:
#ifdef HAVE_GD_JPEG
	    /*
	     * Write IM to OUTFILE as a JFIF-formatted JPEG image, using
	     * quality JPEG_QUALITY.  If JPEG_QUALITY is in the range
	     * 0-100, increasing values represent higher quality but also
	     * larger image size.  If JPEG_QUALITY is negative, the
	     * IJG JPEG library's default quality is used (which should
	     * be near optimal for many applications).  See the IJG JPEG
	     * library documentation for more details.  */
#define JPEG_QUALITY -1
	    gdImageJpegCtx(im, &gd_context.ctx, JPEG_QUALITY);
#endif

	    break;
	case FORMAT_PNG:
#ifdef HAVE_GD_PNG
	    gdImagePngCtx(im, &gd_context.ctx);
#endif
	    break;

	case FORMAT_WBMP:
	    {
	        /* Use black for the foreground color for the B&W wbmp image. */
		int black = gdImageColorResolveAlpha(im, 0, 0, 0, gdAlphaOpaque);
		gdImageWBMPCtx(im, black, &gd_context.ctx);
	    }
	    break;

	case FORMAT_GD:
	    gdImageGd(im, job->output_file);
	    break;

#ifdef HAVE_LIBZ
	case FORMAT_GD2:
#define GD2_CHUNKSIZE 128
#define GD2_COMPRESSED 2
	    gdImageGd2(im, job->output_file, GD2_CHUNKSIZE, GD2_COMPRESSED);
	    break;
#endif

	case FORMAT_XBM:
	    break;
	default:
	    UNREACHABLE();
	}
	gdImageDestroy(im);
	job->context = NULL;
    }
}

/* fontsize at which text is omitted entirely */
#define FONTSIZE_MUCH_TOO_SMALL 0.15
/* fontsize at which text is rendered by a simple line */
#define FONTSIZE_TOO_SMALL 1.5

void gdgen_text(gdImagePtr im, pointf spf, pointf epf, int fontcolor, double fontsize, int fontdpi, double fontangle, char *fontname, char *str)
{
    point sp, ep; /* start point, end point, in pixels */

    PF2P(spf, sp);
    PF2P(epf, ep);

    gdFTStringExtra strex = {
        .flags = gdFTEX_RESOLUTION,
        .hdpi = fontdpi,
        .vdpi = fontdpi,
    };

    if (strchr(fontname, '/'))
        strex.flags |= gdFTEX_FONTPATHNAME;
    else
        strex.flags |= gdFTEX_FONTCONFIG;

    if (fontsize <= FONTSIZE_MUCH_TOO_SMALL) {
        /* ignore entirely */
    } else if (fontsize <= FONTSIZE_TOO_SMALL) {
        /* draw line in place of text */
        gdImageLine(im, sp.x, sp.y, ep.x, ep.y, fontcolor);
    } else {
#ifdef HAVE_GD_FREETYPE
        int brect[8];
#ifdef HAVE_GD_FONTCONFIG
        char* fontlist = fontname;
#else
        extern char *gd_alternate_fontlist(const char *font);
        char* fontlist = gd_alternate_fontlist(fontname);
#endif
        const char *const err = gdImageStringFTEx(im, brect, fontcolor,
                fontlist, fontsize, fontangle, sp.x, sp.y, str, &strex);
#ifndef HAVE_GD_FONTCONFIG
        free(fontlist);
#endif

        if (err) {
            /* revert to builtin fonts */
#endif
            sp.y += 2;
            if (fontsize <= 8.5) {
                gdImageString(im, gdFontTiny, sp.x, sp.y - 9, (unsigned char*)str, fontcolor);
            } else if (fontsize <= 9.5) {
                gdImageString(im, gdFontSmall, sp.x, sp.y - 12, (unsigned char*)str, fontcolor);
            } else if (fontsize <= 10.5) {
                gdImageString(im, gdFontMediumBold, sp.x, sp.y - 13, (unsigned char*)str, fontcolor);
            } else if (fontsize <= 11.5) {
                gdImageString(im, gdFontLarge, sp.x, sp.y - 14, (unsigned char*)str, fontcolor);
            } else {
                gdImageString(im, gdFontGiant, sp.x, sp.y - 15, (unsigned char*)str, fontcolor);
            }
#ifdef HAVE_GD_FREETYPE
        }
#endif
    }
}

static void gdgen_textspan(GVJ_t * job, pointf p, textspan_t * span)
{
    gdImagePtr im = job->context;
    pointf spf, epf;
    double spanwidth = span->size.x * job->zoom * job->dpi.x / POINTS_PER_INCH;
    char* fontname;

    if (!im)
	return;

    switch (span->just) {
    case 'l':
	spf.x = 0.0;
	break;
    case 'r':
	spf.x = -spanwidth;
	break;
    default:
    case 'n':
	spf.x = -spanwidth / 2;
	break;
    }
    epf.x = spf.x + spanwidth;

    if (job->rotation) {
	spf.y = -spf.x + p.y;
	epf.y = epf.x + p.y;
	epf.x = spf.x = p.x;
    }
    else {
	spf.x += p.x;
	epf.x += p.x;
	epf.y = spf.y = p.y - span->yoffset_centerline * job->zoom * job->dpi.x / POINTS_PER_INCH;
    }

    bool fontname_needs_free = false;
#ifdef HAVE_GD_FONTCONFIG
    PostscriptAlias *const pA = span->font->postscript_alias;
    if (pA) {
	fontname = gd_psfontResolve (pA);
	fontname_needs_free = true;
    } else
#endif
	fontname = span->font->name;

    gdgen_text(im, spf, epf,
	    job->obj->pencolor.u.index,
	    span->font->size * job->zoom,
	    d2i(job->dpi.x),
	    job->rotation ? M_PI / 2 : 0,
	    fontname,
	    span->str);
    if (fontname_needs_free) {
	free(fontname);
    }
}

static int gdgen_set_penstyle(GVJ_t * job, gdImagePtr im, gdImagePtr* brush)
{
    obj_state_t *obj = job->obj;
    int i, pen, dashstyle[20];

    if (obj->pen == PEN_DASHED) {
	for (i = 0; i < 10; i++)
	    dashstyle[i] = obj->pencolor.u.index;
	for (; i < 20; i++)
	    dashstyle[i] = gdTransparent;
	gdImageSetStyle(im, dashstyle, 20);
	pen = gdStyled;
    } else if (obj->pen == PEN_DOTTED) {
	for (i = 0; i < 2; i++)
	    dashstyle[i] = obj->pencolor.u.index;
	for (; i < 12; i++)
	    dashstyle[i] = gdTransparent;
	gdImageSetStyle(im, dashstyle, 12);
	pen = gdStyled;
    } else {
	pen = obj->pencolor.u.index;
    }

    int width = d2i(obj->penwidth * job->zoom);
    if (width < PENWIDTH_NORMAL)
	width = PENWIDTH_NORMAL;  /* gd can't do thin lines */
    gdImageSetThickness(im, width);
    /* use brush instead of Thickness to improve end butts */
    if (width != (int)PENWIDTH_NORMAL) {
	if (im->trueColor) {
	    *brush = gdImageCreateTrueColor(width,width);
	}
	else {
	    *brush = gdImageCreate(width, width);
	    gdImagePaletteCopy(*brush, im);
	}
	gdImageFilledRectangle(*brush, 0, 0, width - 1, width - 1,
			       obj->pencolor.u.index);
	gdImageSetBrush(im, *brush);
	if (pen == gdStyled)
	    pen = gdStyledBrushed;
	else
	    pen = gdBrushed;
    }

    return pen;
}

static void gdgen_bezier(GVJ_t *job, pointf *A, size_t n, int filled) {
    obj_state_t *obj = job->obj;
    gdImagePtr im = job->context;
    pointf V[4];
    gdImagePtr brush = NULL;
    gdPoint F[4];

    if (!im)
	return;

    const int pen = gdgen_set_penstyle(job, im, &brush);
    const bool pen_ok = pen != gdImageGetTransparent(im);
    const bool fill_ok = filled && obj->fillcolor.u.index != gdImageGetTransparent(im);

    if (pen_ok || fill_ok) {
        V[3] = A[0];
        PF2P(A[0], F[0]);
        PF2P(A[n-1], F[3]);
        for (size_t i = 0; i + 3 < n; i += 3) {
	    V[0] = V[3];
	    for (size_t j = 1; j <= 3; j++)
	        V[j] = A[i + j];
	    pointf p0 = V[0];
	    for (int step = 1; step <= BEZIERSUBDIVISION; step++) {
	        const pointf p1 = Bezier(V, (double)step / BEZIERSUBDIVISION, NULL, NULL);
	        PF2P(p0, F[1]);
	        PF2P(p1, F[2]);
	        if (pen_ok)
	            gdImageLine(im, F[1].x, F[1].y, F[2].x, F[2].y, pen);
	        if (fill_ok)
		    gdImageFilledPolygon(im, F, 4, obj->fillcolor.u.index);
	        p0 = p1;
	    }
        }
    }
    if (brush)
	gdImageDestroy(brush);
}

static gdPoint *points;
static size_t points_allocated;

static void gdgen_polygon(GVJ_t *job, pointf *A, size_t n, int filled) {
    obj_state_t *obj = job->obj;
    gdImagePtr im = job->context;
    gdImagePtr brush = NULL;

    if (!im)
	return;

    const int pen = gdgen_set_penstyle(job, im, &brush);
    const bool pen_ok = pen != gdImageGetTransparent(im);
    const bool fill_ok = filled && obj->fillcolor.u.index != gdImageGetTransparent(im);

    if (pen_ok || fill_ok) {
        if (n > points_allocated) {
	    points = gv_recalloc(points, points_allocated, n, sizeof(gdPoint));
	    points_allocated = n;
	}
        for (size_t i = 0; i < n; i++) {
	    points[i].x = ROUND(A[i].x);
	    points[i].y = ROUND(A[i].y);
        }
        assert(n <= INT_MAX);
        if (fill_ok)
	    gdImageFilledPolygon(im, points, (int)n, obj->fillcolor.u.index);
    
        if (pen_ok)
            gdImagePolygon(im, points, (int)n, pen);
    }
    if (brush)
	gdImageDestroy(brush);
}

static void gdgen_ellipse(GVJ_t * job, pointf * A, int filled)
{
    obj_state_t *obj = job->obj;
    gdImagePtr im = job->context;
    gdImagePtr brush = NULL;

    if (!im)
	return;

    const int pen = gdgen_set_penstyle(job, im, &brush);
    const bool pen_ok = pen != gdImageGetTransparent(im);
    const bool fill_ok = filled && obj->fillcolor.u.index != gdImageGetTransparent(im);

    const double dx = 2 * (A[1].x - A[0].x);
    const double dy = 2 * (A[1].y - A[0].y);

    if (fill_ok)
	gdImageFilledEllipse(im, ROUND(A[0].x), ROUND(A[0].y),
			     ROUND(dx), ROUND(dy),
			     obj->fillcolor.u.index);
    if (pen_ok)
        gdImageArc(im, ROUND(A[0].x), ROUND(A[0].y), ROUND(dx), ROUND(dy),
	           0, 360, pen);
    if (brush)
	gdImageDestroy(brush);
}

static void gdgen_polyline(GVJ_t *job, pointf *A, size_t n) {
    gdImagePtr im = job->context;
    gdImagePtr brush = NULL;

    if (!im)
	return;

    const int pen = gdgen_set_penstyle(job, im, &brush);
    const bool pen_ok = pen != gdImageGetTransparent(im);

    if (pen_ok) {
        pointf p = A[0];
        for (size_t i = 1; i < n; i++) {
	    const pointf p1 = A[i];
	    gdImageLine(im, ROUND(p.x), ROUND(p.y),
		        ROUND(p1.x), ROUND(p1.y), pen);
	    p = p1;
        }
    }
    if (brush)
	gdImageDestroy(brush);
}

static gvrender_engine_t gdgen_engine = {
    0,				/* gdgen_begin_job */
    0,				/* gdgen_end_job */
    0,				/* gdgen_begin_graph */
    0,				/* gdgen_end_graph */
    0,				/* gdgen_begin_layer */
    0,				/* gdgen_end_layer */
    gdgen_begin_page,
    gdgen_end_page,
    0,				/* gdgen_begin_cluster */
    0,				/* gdgen_end_cluster */
    0,				/* gdgen_begin_nodes */
    0,				/* gdgen_end_nodes */
    0,				/* gdgen_begin_edges */
    0,				/* gdgen_end_edges */
    0,				/* gdgen_begin_node */
    0,				/* gdgen_end_node */
    0,				/* gdgen_begin_edge */
    0,				/* gdgen_end_edge */
    0,				/* gdgen_begin_anchor */
    0,				/* gdgen_end_anchor */
    0,				/* gdgen_begin_label */
    0,				/* gdgen_end_label */
    gdgen_textspan,
    gdgen_resolve_color,
    gdgen_ellipse,
    gdgen_polygon,
    gdgen_bezier,
    gdgen_polyline,
    0,				/* gdgen_comment */
    0,				/* gdgen_library_shape */
};

static gvrender_features_t render_features_gd = {
    GVRENDER_Y_GOES_DOWN,	/* flags */
    4.,                         /* default pad - graph units */
    NULL,			/* knowncolors */
    0,				/* sizeof knowncolors */
    RGBA_BYTE,			/* color_type */
};

static gvdevice_features_t device_features_gd = {
    GVDEVICE_BINARY_FORMAT,	/* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},			/* default dpi */
};

static gvdevice_features_t device_features_gd_tc = {
    GVDEVICE_BINARY_FORMAT
      | GVDEVICE_DOES_TRUECOLOR,/* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},			/* default dpi */
};

static gvdevice_features_t device_features_gd_tc_no_writer = {
    GVDEVICE_BINARY_FORMAT
      | GVDEVICE_DOES_TRUECOLOR
      | GVDEVICE_NO_WRITER,	/* flags */
    {0.,0.},			/* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},			/* default dpi */
};

gvplugin_installed_t gvrender_gd_types[] = {
    {FORMAT_GD, "gd", 1, &gdgen_engine, &render_features_gd},
    {0, NULL, 0, NULL, NULL}
};

gvplugin_installed_t gvdevice_gd_types2[] = {
    {FORMAT_GIF, "gif:gd", 1, NULL, &device_features_gd_tc},  /* pretend gif is truecolor because it supports transparency */
    {FORMAT_WBMP, "wbmp:gd", 1, NULL, &device_features_gd},

#ifdef HAVE_GD_JPEG
    {FORMAT_JPEG, "jpe:gd", 1, NULL, &device_features_gd},
    {FORMAT_JPEG, "jpeg:gd", 1, NULL, &device_features_gd},
    {FORMAT_JPEG, "jpg:gd", 1, NULL, &device_features_gd},
#endif

#ifdef HAVE_GD_PNG
    {FORMAT_PNG, "png:gd", 1, NULL, &device_features_gd_tc},
#endif

    {FORMAT_GD, "gd:gd", 1, NULL, &device_features_gd_tc_no_writer},
    
#ifdef HAVE_LIBZ
    {FORMAT_GD2, "gd2:gd", 1, NULL, &device_features_gd_tc_no_writer},
#endif

    {0, NULL, 0, NULL, NULL}
};
