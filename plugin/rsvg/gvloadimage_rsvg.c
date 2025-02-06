/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <assert.h>
#include <common/render.h>
#include <common/utils.h>
#include <stdbool.h>
#include <stddef.h>

#include <gvc/gvplugin_loadimage.h>

#include <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif
#include <cairo-svg.h>

enum {
    FORMAT_SVG_CAIRO,
};

static void gvloadimage_rsvg_free(usershape_t *us)
{
  g_object_unref(us->data);
}

static RsvgHandle* gvloadimage_rsvg_load(GVJ_t * job, usershape_t *us)
{
    RsvgHandle* rsvgh = NULL;
    GError *err = NULL;

    assert(job);
    assert(us);
    assert(us->name);

    if (us->data) {
        if (us->datafree == gvloadimage_rsvg_free)
             rsvgh = us->data; /* use cached data */
        else {
             us->datafree(us);        /* free incompatible cache data */
             us->data = NULL;
        }

    }

    if (!rsvgh) { /* read file into cache */
	if (!gvusershape_file_access(us))
	    return NULL;
        switch (us->type) {
            case FT_SVG: {
      		const char *const safe_path = safefile(us->name);
      		assert(safe_path != NULL &&
      		       "gvusershape_file_access did not validate file path");
      		rsvgh = rsvg_handle_new_from_file(safe_path, &err);
		
		if (rsvgh == NULL) {
			fprintf(stderr, "rsvg_handle_new_from_file returned an error: %s\n", err->message);
			g_error_free(err);
			return NULL;
		} 

		rsvg_handle_set_dpi(rsvgh, POINTS_PER_INCH);

                break;
            }
            default:
                rsvgh = NULL;
        }

        if (rsvgh) {
            us->data = rsvgh;
            us->datafree = gvloadimage_rsvg_free;
        }

	gvusershape_file_release(us);
    }

    return rsvgh;
}

static void gvloadimage_rsvg_cairo(GVJ_t * job, usershape_t *us, boxf b, bool filled)
{
    (void)filled;

    RsvgHandle* rsvgh = gvloadimage_rsvg_load(job, us);

    cairo_t *cr = job->context; /* target context */
    cairo_surface_t *surface;	 /* source surface */

    if (rsvgh) {
        cairo_save(cr);

       	surface = cairo_svg_surface_create(NULL, us->w, us->h); 

	cairo_surface_reference(surface);

        cairo_set_source_surface(cr, surface, 0, 0);
	cairo_translate(cr, b.LL.x, -b.UR.y);
	cairo_scale(cr, (b.UR.x - b.LL.x) / us->w, (b.UR.y - b.LL.y) / us->h);
#if LIBRSVG_MAJOR_VERSION > 2 || (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION >= 46)
	const RsvgRectangle viewport = {.width = b.UR.x - b.LL.x,
	                                .height = b.UR.y - b.LL.y};
	rsvg_handle_render_document(rsvgh, cr, &viewport, NULL);
#else
	rsvg_handle_render_cairo(rsvgh, cr);
#endif

        cairo_paint (cr);
        cairo_restore(cr);
    }
}

static gvloadimage_engine_t engine_cairo = {
    gvloadimage_rsvg_cairo
};

gvplugin_installed_t gvloadimage_rsvg_types[] = {
    {FORMAT_SVG_CAIRO, "svg:cairo", 1, &engine_cairo, NULL},
    {0, NULL, 0, NULL, NULL}
};
