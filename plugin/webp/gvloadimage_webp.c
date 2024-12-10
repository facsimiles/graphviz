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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/gv_ftell.h>
#include <util/gv_math.h>
#include <util/prisize_t.h>

#include <gvc/gvplugin_loadimage.h>
#include <gvc/gvio.h>

#ifdef HAVE_WEBP
#ifdef HAVE_PANGOCAIRO
#include <cairo.h>
#include <webp/decode.h>

static const char* const kStatusMessages[] = {
    "OK", "OUT_OF_MEMORY", "INVALID_PARAM", "BITSTREAM_ERROR",
    "UNSUPPORTED_FEATURE", "SUSPENDED", "USER_ABORT", "NOT_ENOUGH_DATA"
};

enum {
    FORMAT_WEBP_CAIRO,
};

static void webp_freeimage(usershape_t *us)
{
    cairo_surface_destroy(us->data);
}

static cairo_surface_t* webp_really_loadimage(const char *in_file, FILE* const in)
{
    WebPDecoderConfig config;
    WebPDecBuffer* const output_buffer = &config.output;
    WebPBitstreamFeatures* const bitstream = &config.input;
    VP8StatusCode status = VP8_STATUS_OK;
    cairo_surface_t *surface = NULL; /* source surface */
    int ok;
    void* data = NULL;

    if (!WebPInitDecoderConfig(&config)) {
	fprintf(stderr, "Error: WebP library version mismatch!\n");
	return NULL;
    }

    fseek(in, 0, SEEK_END);
    const size_t data_size = gv_ftell(in);
    rewind(in);
    data = malloc(data_size);
    ok = data_size == 0 || (data != NULL && fread(data, data_size, 1, in) == 1);
    if (!ok) {
        fprintf(stderr, "Error: WebP could not read %" PRISIZE_T
                " bytes of data from %s\n", data_size, in_file);
        free(data);
        return NULL;
    }

    status = WebPGetFeatures(data, data_size, bitstream);
    if (status != VP8_STATUS_OK) {
	goto end;
    }

    output_buffer->colorspace = MODE_RGBA;
    status = WebPDecode(data, data_size, &config);

    /* FIXME - this is ugly */
    if (! bitstream->has_alpha) {
	assert(output_buffer->width >= 0);
	assert(output_buffer->height >= 0);
	argb2rgba((size_t)output_buffer->width, (size_t)output_buffer->height,
	          output_buffer->u.RGBA.rgba);
    }

end:
    free(data);
    ok = status == VP8_STATUS_OK;
    if (!ok) {
	fprintf(stderr, "Error: WebP decoding of %s failed.\n", in_file);
	fprintf(stderr, "Status: %d (%s)\n", status, kStatusMessages[status]);
	return NULL;
    }

    surface = cairo_image_surface_create_for_data (
	    output_buffer->u.RGBA.rgba,
	    CAIRO_FORMAT_ARGB32,
	    output_buffer->width,
	    output_buffer->height,
	    output_buffer->u.RGBA.stride);

    return surface;
}

// get image either from cached surface, or from freshly loaded surface
static cairo_surface_t *webp_loadimage(usershape_t *us) {
    cairo_surface_t *surface = NULL; /* source surface */

    assert(us);
    assert(us->name);

    if (us->data) {
        if (us->datafree == webp_freeimage)
             surface = us->data; /* use cached data */
        else {
             us->datafree(us);        /* free incompatible cache data */
             us->datafree = NULL;
             us->data = NULL;
        }
    }
    if (!surface) { /* read file into cache */
	if (!gvusershape_file_access(us))
	    return NULL;
        switch (us->type) {
            case FT_WEBP:
		if ((surface = webp_really_loadimage(us->name, us->f)))
                    cairo_surface_reference(surface);
                break;
            default:
                surface = NULL;
        }
        if (surface) {
            us->data = surface;
            us->datafree = webp_freeimage;
        }
	gvusershape_file_release(us);
    }
    return surface;
}

/* paint image into required location in graph */
static void webp_loadimage_cairo(GVJ_t * job, usershape_t *us, boxf b, bool filled)
{
    (void)filled;

    cairo_t *cr = job->context; /* target context */
    cairo_surface_t *surface;	 /* source surface */

    surface = webp_loadimage(us);
    if (surface) {
        cairo_save(cr);
	cairo_translate(cr, b.LL.x, -b.UR.y);
	cairo_scale(cr, (b.UR.x - b.LL.x) / us->w, (b.UR.y - b.LL.y) / us->h);
        cairo_set_source_surface (cr, surface, 0, 0);
        cairo_paint (cr);
        cairo_restore(cr);
    }
}

static gvloadimage_engine_t engine_webp = {
    webp_loadimage_cairo
};
#endif
#endif

gvplugin_installed_t gvloadimage_webp_types[] = {
#ifdef HAVE_WEBP
#ifdef HAVE_PANGOCAIRO
    {FORMAT_WEBP_CAIRO, "webp:cairo", 1, &engine_webp, NULL},
#endif
#endif
    {0, NULL, 0, NULL, NULL}
};
