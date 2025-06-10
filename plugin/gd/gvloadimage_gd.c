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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <util/alloc.h>

#ifdef HAVE_PANGOCAIRO
#include <cairo.h>
#endif

#include <gvc/gvplugin_loadimage.h>
#include <gvc/gvio.h>
#include <gd.h>

enum {
    FORMAT_PNG_GD, FORMAT_GIF_GD, FORMAT_JPG_GD, FORMAT_GD_GD, FORMAT_GD2_GD, FORMAT_XPM_GD, FORMAT_WBMP_GD, FORMAT_XBM_GD,
    FORMAT_PNG_PS, FORMAT_GIF_PS, FORMAT_JPG_PS, FORMAT_GD_PS, FORMAT_GD2_PS, FORMAT_XPM_PS, FORMAT_WBMP_PS, FORMAT_XBM_PS,
    FORMAT_PNG_CAIRO, FORMAT_GIF_CAIRO, FORMAT_JPG_CAIRO, FORMAT_GD_CAIRO, FORMAT_GD2_CAIRO, FORMAT_XPM_CAIRO, FORMAT_WBMP_CAIRO, FORMAT_XBM_CAIRO,
};

static void gd_freeimage(usershape_t *us)
{
    gdImageDestroy(us->data);
}

static gdImagePtr gd_loadimage(usershape_t *us) {
    assert(us);
    assert(us->name);

    if (us->data) {
	if (us->datafree != gd_freeimage) {
	     us->datafree(us);        /* free incompatible cache data */
	     us->data = NULL;
	     us->datafree = NULL;
	}
    }
    if (!us->data) { /* read file into cache */
	if (!gvusershape_file_access(us))
	    return NULL;
	switch (us->type) {
#ifdef HAVE_GD_PNG
	    case FT_PNG:
		us->data = gdImageCreateFromPng(us->f);
		break;
#endif
#ifdef HAVE_GD_GIF
	    case FT_GIF:
		us->data = gdImageCreateFromGif(us->f);
		break;
#endif
#ifdef HAVE_GD_JPEG
	    case FT_JPEG:
		us->data = gdImageCreateFromJpeg(us->f);
		break;
#endif
	    default:
		break;
	}
        if (us->data)
	    us->datafree = gd_freeimage;

	gvusershape_file_release(us);
    }
    return us->data;
}

static gdImagePtr gd_rotateimage(gdImagePtr im, int rotation)
{
    gdImagePtr im2 = gdImageCreate(im->sy, im->sx);

    gdImageCopyRotated(im2, im, im2->sx / 2., im2->sy / 2.,
                0, 0, im->sx, im->sy, rotation);
    gdImageDestroy(im);
    return im2;
}
	
static void gd_loadimage_gd(GVJ_t * job, usershape_t *us, boxf b, bool filled)
{
    (void)filled;

    gdImagePtr im2, im = job->context;

    if ((im2 = gd_loadimage(us))) {
        if (job->rotation) {
	    im2 = gd_rotateimage(im2, job->rotation);
	    us->data = im2;
        }
        gdImageCopyResized(im, im2, ROUND(b.LL.x), ROUND(b.LL.y), 0, 0,
                ROUND(b.UR.x - b.LL.x), ROUND(b.UR.y - b.LL.y), im2->sx, im2->sy);
    }
}

#ifdef HAVE_PANGOCAIRO
static void gd_loadimage_cairo(GVJ_t * job, usershape_t *us, boxf b, bool filled)
{
    (void)filled;

    cairo_t *cr = job->context; /* target context */
    int x, y, width, height;
    cairo_surface_t *surface;    /* source surface */
    gdImagePtr im;

    if ((im = gd_loadimage(us))) {
	width = im->sx;
	height = im->sy;
        const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
                                                         width);
	assert(stride >= 0);
	assert(height >= 0);
	unsigned char *data = gv_calloc((size_t)stride, (size_t)height);
	surface = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_ARGB32,
							width, height, stride);
	unsigned char *const orig_data = data;

	if (im->trueColor) {
	    if (im->saveAlphaFlag) {
	        for (y = 0; y < height; y++) {
		    for (x = 0; x < width; x++) {
		        const int px = gdImageTrueColorPixel(im, x, y);
		        *data++ = (unsigned char)gdTrueColorGetBlue(px);
		        *data++ = (unsigned char)gdTrueColorGetGreen(px);
		        *data++ = (unsigned char)gdTrueColorGetRed(px);
		        // gd’s alpha is 7-bit, so scale up ×2 to our 8-bit
		        *data++ = (unsigned char)((0x7F - gdTrueColorGetAlpha(px)) << 1);
		    }
		}
	    }
	    else {
	        for (y = 0; y < height; y++) {
		    for (x = 0; x < width; x++) {
		        const int px = gdImageTrueColorPixel(im, x, y);
		        *data++ = (unsigned char)gdTrueColorGetBlue(px);
		        *data++ = (unsigned char)gdTrueColorGetGreen(px);
		        *data++ = (unsigned char)gdTrueColorGetRed(px);
		        *data++ = 0xFF;
		    }
		}
	    }
	}
	else {
	    for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
		    const int px = gdImagePalettePixel(im, x, y);
		    *data++ = (unsigned char)im->blue[px];
		    *data++ = (unsigned char)im->green[px];
		    *data++ = (unsigned char)im->red[px];
		    *data++ = px == im->transparent ? 0x00 : 0xff;
		}
	    }
	}

        cairo_save(cr);
        cairo_translate(cr, b.LL.x, -b.UR.y);
        cairo_scale(cr, (b.UR.x - b.LL.x) / us->w, (b.UR.y - b.LL.y) / us->h);
        cairo_set_source_surface (cr, surface, 0, 0);
        cairo_paint (cr);
        cairo_restore(cr);

	cairo_surface_destroy(surface);
	free(orig_data);
    }
}
#endif

static void gd_loadimage_ps(GVJ_t * job, usershape_t *us, boxf b, bool filled)
{
    (void)filled;

    gdImagePtr im = NULL;
    int X, Y, x, y, px;

    if ((im = gd_loadimage(us))) {
	X = im->sx;
	Y = im->sy;

        gvputs(job, "save\n");

	/* define image data as string array (one per raster line) */
        gvputs(job, "/myctr 0 def\n");
        gvputs(job, "/myarray [\n");
        if (im->trueColor) {
            for (y = 0; y < Y; y++) {
		gvputs(job, "<");
                for (x = 0; x < X; x++) {
                    px = gdImageTrueColorPixel(im, x, y);
                    gvprintf(job, "%02x%02x%02x",
                        gdTrueColorGetRed(px),
                        gdTrueColorGetGreen(px),
                        gdTrueColorGetBlue(px));
                }
		gvputs(job, ">\n");
            }
	}
        else {
            for (y = 0; y < Y; y++) {
		gvputs(job, "<");
                for (x = 0; x < X; x++) {
                    px = gdImagePalettePixel(im, x, y);
                    gvprintf(job, "%02x%02x%02x",
                        im->red[px],
                        im->green[px],
                        im->blue[px]);
                }
		gvputs(job, ">\n");
            }
        }
	gvputs(job, "] def\n");
	gvputs(job,"/myproc { myarray myctr get /myctr myctr 1 add def } def\n");

        /* this sets the position of the image */
        gvprintf(job, "%g %g translate\n",
		b.LL.x + (b.UR.x - b.LL.x) * (1. - (job->dpi.x) / 96.) / 2.,
	        b.LL.y + (b.UR.y - b.LL.y) * (1. - (job->dpi.y) / 96.) / 2.);

        /* this sets the rendered size to fit the box */
        gvprintf(job,"%g %g scale\n",
		(b.UR.x - b.LL.x) * (job->dpi.x) / 96.,
		(b.UR.y - b.LL.y) * (job->dpi.y) / 96.);
    
        /* xsize ysize bits-per-sample [matrix] */
        gvprintf(job, "%d %d 8 [%d 0 0 %d 0 %d]\n", X, Y, X, -Y, Y);
    
        gvputs(job, "{myproc} false 3 colorimage\n");
    
        gvputs(job, "restore\n");
    }
}

static gvloadimage_engine_t engine = {
    gd_loadimage_gd
};

static gvloadimage_engine_t engine_ps = {
    gd_loadimage_ps
};

#ifdef HAVE_PANGOCAIRO
static gvloadimage_engine_t engine_cairo = {
    gd_loadimage_cairo
};
#endif

gvplugin_installed_t gvloadimage_gd_types[] = {
    {FORMAT_GD_GD, "gd:gd", 1, &engine, NULL},
    {FORMAT_GD2_GD, "gd2:gd", 1, &engine, NULL},
#ifdef HAVE_GD_GIF
    {FORMAT_GIF_GD, "gif:gd", 1, &engine, NULL},
#endif
#ifdef HAVE_GD_JPEG
    {FORMAT_JPG_GD, "jpeg:gd", 1, &engine, NULL},
    {FORMAT_JPG_GD, "jpe:gd", 1, &engine, NULL},
    {FORMAT_JPG_GD, "jpg:gd", 1, &engine, NULL},
#endif
#ifdef HAVE_GD_PNG
    {FORMAT_PNG_GD, "png:gd", 1, &engine, NULL},
#endif
#ifdef HAVE_GD_WBMP
    {FORMAT_WBMP_GD, "wbmp:gd", 1, &engine, NULL},
#endif
#ifdef HAVE_GD_XPM
    {FORMAT_XBM_GD, "xbm:gd", 1, &engine, NULL},
#endif

    {FORMAT_GD_PS, "gd:ps", 1, &engine_ps, NULL},
    {FORMAT_GD_PS, "gd:lasi", 1, &engine_ps, NULL},
    {FORMAT_GD2_PS, "gd2:ps", 1, &engine_ps, NULL},
    {FORMAT_GD2_PS, "gd2:lasi", 1, &engine_ps, NULL},
#ifdef HAVE_GD_GIF
    {FORMAT_GIF_PS, "gif:ps", 1, &engine_ps, NULL},
    {FORMAT_GIF_PS, "gif:lasi", 1, &engine_ps, NULL},
#endif
#ifdef HAVE_GD_JPEG
    {FORMAT_JPG_PS, "jpeg:ps", 1, &engine_ps, NULL},
    {FORMAT_JPG_PS, "jpg:ps", 1, &engine_ps, NULL},
    {FORMAT_JPG_PS, "jpe:ps", 1, &engine_ps, NULL},
    {FORMAT_JPG_PS, "jpeg:lasi", 1, &engine_ps, NULL},
    {FORMAT_JPG_PS, "jpg:lasi", 1, &engine_ps, NULL},
    {FORMAT_JPG_PS, "jpe:lasi", 1, &engine_ps, NULL},
#endif
#ifdef HAVE_GD_PNG
    {FORMAT_PNG_PS, "png:ps", 1, &engine_ps, NULL},
    {FORMAT_PNG_PS, "png:lasi", 1, &engine_ps, NULL},
#endif
#ifdef HAVE_GD_WBMP
    {FORMAT_WBMP_PS, "wbmp:ps", 1, &engine_ps, NULL},
    {FORMAT_WBMP_PS, "wbmp:lasi", 1, &engine_ps, NULL},
#endif
#ifdef HAVE_GD_XPM
    {FORMAT_XBM_PS, "xbm:ps", 1, &engine_ps, NULL},
    {FORMAT_XBM_PS, "xbm:lasi", 1, &engine_ps, NULL},
#endif

#ifdef HAVE_PANGOCAIRO
    {FORMAT_GD_CAIRO, "gd:cairo", 1, &engine_cairo, NULL},
    {FORMAT_GD2_CAIRO, "gd2:cairo", 1, &engine_cairo, NULL},
#ifdef HAVE_GD_GIF
    {FORMAT_GIF_CAIRO, "gif:cairo", 1, &engine_cairo, NULL},
#endif
#ifdef HAVE_GD_JPEG
    {FORMAT_JPG_CAIRO, "jpeg:cairo", 1, &engine_cairo, NULL},
    {FORMAT_JPG_CAIRO, "jpg:cairo", 1, &engine_cairo, NULL},
    {FORMAT_JPG_CAIRO, "jpe:cairo", 1, &engine_cairo, NULL},
#endif
#ifdef HAVE_GD_PNG
    {FORMAT_PNG_CAIRO, "png:cairo", -1, &engine_cairo, NULL},
#endif
#ifdef HAVE_GD_WBMP
    {FORMAT_WBMP_CAIRO, "wbmp:cairo", 1, &engine_cairo, NULL},
#endif
#ifdef HAVE_GD_XPM
    {FORMAT_XBM_CAIRO, "xbm:cairo", 1, &engine_cairo, NULL},
#endif
#endif /* HAVE_PANGOCAIRO */
    {0, NULL, 0, NULL, NULL}
};
