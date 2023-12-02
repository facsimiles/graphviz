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
#include "gdioctx_wrapper.h"

#include <gvc/gvplugin_device.h>
#include <gvc/gvio.h>

#include <gd.h>
#include <stddef.h>

int gvdevice_gd_putBuf (gdIOCtx *context, const void *buffer, int len)
{
    gd_context_t *gd_context = get_containing_context(context);
    return gvwrite(gd_context->job, buffer, len);
}

/* used by gif output */
void gvdevice_gd_putC (gdIOCtx *context, int C)
{
    gd_context_t *gd_context = get_containing_context(context);
    char c = C;

    gvwrite(gd_context->job, &c, 1);
}

#ifdef HAVE_PANGOCAIRO
typedef enum {
	FORMAT_GIF,
	FORMAT_JPEG,
	FORMAT_PNG,
	FORMAT_WBMP,
	FORMAT_GD,
	FORMAT_GD2,
	FORMAT_XBM,
} format_type;

static void gd_format(GVJ_t * job)
{
    gdImagePtr im;
    unsigned int x, y, color, alpha;
    unsigned int *data = (unsigned int*)job->imagedata;
    unsigned int width = job->width;
    unsigned int height = job->height;
    gd_context_t gd_context = {{0}};

    gd_context.ctx.putBuf = gvdevice_gd_putBuf;
    gd_context.ctx.putC = gvdevice_gd_putC;
    gd_context.job = job;

    im = gdImageCreateTrueColor(width, height);
    switch (job->device.id) {
#ifdef HAVE_GD_PNG
    case FORMAT_PNG:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                color = *data++;
	        /* gd's max alpha is 127 */
	        /*   so right-shift 25 to lose lsb of alpha */
	        alpha = (color >> 25) & 0x7f;
	        im->tpixels[y][x] = (color & 0xffffff) | ((0x7f - alpha) << 24);
	    }
        }
        break;
#endif
    default:
/* pick an off-white color, so that transparent backgrounds look white in jpgs */
#define TRANSPARENT 0x7ffffffe

        gdImageColorTransparent(im, TRANSPARENT);
        gdImageAlphaBlending(im, FALSE);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                color = *data++;
	        /* gd's max alpha is 127 */
	        /*   so right-shift 25 to lose lsb of alpha */
	        if ((alpha = (color >> 25) & 0x7f) >= 0x20)
		    /* if not > 75% transparent */
		    im->tpixels[y][x] = (color & 0xffffff) | ((0x7f - alpha) << 24);
	        else
		    im->tpixels[y][x] = TRANSPARENT;
	    }
        }
        break;
    }

    switch (job->device.id) {
#ifdef HAVE_GD_GIF
    case FORMAT_GIF:
	gdImageTrueColorToPalette(im, 0, 256);
	gdImageGifCtx(im, &gd_context.ctx);
        break;
#endif

#ifdef HAVE_GD_JPEG
    case FORMAT_JPEG:
	/*
	 * Write IM to OUTFILE as a JFIF-formatted JPEG image, using
	 * quality JPEG_QUALITY.  If JPEG_QUALITY is in the range
	 * 0-100, increasing values represent higher quality but also
	 * larger image size.  If JPEG_QUALITY is negative, the
	 * IJG JPEG library's default quality is used (which should
	 * be near optimal for many applications).  See the IJG JPEG
	 * library documentation for more details.
	 */ 
#define JPEG_QUALITY -1
	gdImageJpegCtx(im, &gd_context.ctx, JPEG_QUALITY);
	break;
#endif

#ifdef HAVE_GD_PNG
    case FORMAT_PNG:
        gdImageTrueColorToPalette(im, 0, 256);
	gdImagePngCtx(im, &gd_context.ctx);
        break;
#endif

    case FORMAT_GD:
	gdImageGd(im, job->output_file);
	break;

    case FORMAT_GD2:
#define GD2_CHUNKSIZE 128
#define GD2_RAW 1
#define GD2_COMPRESSED 2
	gdImageGd2(im, job->output_file, GD2_CHUNKSIZE, GD2_COMPRESSED);
	break;

#ifdef HAVE_GD_GIF
    case FORMAT_WBMP:
	{
	    /* Use black for the foreground color for the B&W wbmp image. */
            int black = gdImageColorResolveAlpha(im, 0, 0, 0, gdAlphaOpaque);
	    gdImageWBMPCtx(im, black, &gd_context.ctx);
	}
	break;
#endif

	break;
    default:
	break;
    }

    gdImageDestroy(im);
}

static gvdevice_engine_t gd_engine = {
    NULL,		/* gd_initialize */
    gd_format,
    NULL,		/* gd_finalize */
};

static gvdevice_features_t device_features_gd = {
    GVDEVICE_BINARY_FORMAT
      | GVDEVICE_DOES_TRUECOLOR,/* flags */
    {0.,0.},                    /* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},                  /* dpi */
};

static gvdevice_features_t device_features_gd_no_writer = {
    GVDEVICE_BINARY_FORMAT
      | GVDEVICE_NO_WRITER
      | GVDEVICE_DOES_TRUECOLOR,/* flags */
    {0.,0.},                    /* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},                  /* dpi */
};
#endif

gvplugin_installed_t gvdevice_gd_types[] = {
#ifdef HAVE_PANGOCAIRO

#ifdef HAVE_GD_GIF
    {FORMAT_GIF, "gif:cairo", 10, &gd_engine, &device_features_gd},
    {FORMAT_WBMP, "wbmp:cairo", 5, &gd_engine, &device_features_gd},
#endif

#ifdef HAVE_GD_JPEG
    {FORMAT_JPEG, "jpe:cairo", 5, &gd_engine, &device_features_gd},
    {FORMAT_JPEG, "jpeg:cairo", 5, &gd_engine, &device_features_gd},
    {FORMAT_JPEG, "jpg:cairo", 5, &gd_engine, &device_features_gd},
#endif

#ifdef HAVE_GD_PNG
    {FORMAT_PNG, "png:cairo", 5, &gd_engine, &device_features_gd},
#endif

    {FORMAT_GD, "gd:cairo", 5, &gd_engine, &device_features_gd_no_writer},
    {FORMAT_GD2, "gd2:cairo", 5, &gd_engine, &device_features_gd_no_writer},

#endif
    {0, NULL, 0, NULL, NULL}
};
