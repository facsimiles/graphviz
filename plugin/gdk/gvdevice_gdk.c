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
#include <gvc/gvplugin_device.h>
#include <gvc/gvio.h>
#include <limits.h>
#include <util/gv_math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <util/agxbuf.h>

enum {
	FORMAT_BMP,
	FORMAT_ICO,
	FORMAT_JPEG,
	FORMAT_PNG,
	FORMAT_TIFF,
};

static gboolean writer(const char *buf, gsize count, GError **error,
                       void *data) {
  (void)error;
  return count == gvwrite(data, buf, count);
}

static void gdk_format(GVJ_t * job)
{
    char *const format_strs[] = {
	[FORMAT_BMP] = "bmp",
	[FORMAT_ICO] = "ico",
	[FORMAT_JPEG] = "jpeg",
	[FORMAT_PNG] = "png",
	[FORMAT_TIFF] = "tiff"
    };
    assert(job->device.id >= 0);
    assert(job->device.id < (int)(sizeof(format_strs) / sizeof(format_strs[0])));
    char *const format_str = format_strs[job->device.id];
    GdkPixbuf *pixbuf;

    argb2rgba(job->width, job->height, job->imagedata);

    assert(job->width <= INT_MAX / BYTES_PER_PIXEL && "width out of range");
    assert(job->height <= INT_MAX && "height out of range");

    pixbuf = gdk_pixbuf_new_from_data(
                job->imagedata,         // data
                GDK_COLORSPACE_RGB,     // colorspace
                TRUE,                   // has_alpha
                8,                      // bits_per_sample
                (int)job->width,        // width
                (int)job->height,       // height
                BYTES_PER_PIXEL * (int)job->width, // rowstride
                NULL,                   // destroy_fn
                NULL                    // destroy_fn_data
               );

    agxbuf x_dpi = {0};
    agxbprint(&x_dpi, "%.0f", job->dpi.x);
    agxbuf y_dpi = {0};
    agxbprint(&y_dpi, "%.0f", job->dpi.y);

    gdk_pixbuf_save_to_callback(pixbuf, writer, job, format_str, NULL, "x-dpi",
                                agxbuse(&x_dpi), "y-dpi", agxbuse(&y_dpi),
                                NULL);

    agxbfree(&y_dpi);
    agxbfree(&x_dpi);
    g_object_unref(pixbuf);
}

static gvdevice_engine_t gdk_engine = {
    NULL,		/* gdk_initialize */
    gdk_format,
    NULL,		/* gdk_finalize */
};

static gvdevice_features_t device_features_gdk = {
    GVDEVICE_BINARY_FORMAT
      | GVDEVICE_DOES_TRUECOLOR,/* flags */
    {0.,0.},                    /* default margin - points */
    {0.,0.},                    /* default page width, height - points */
    {96.,96.},                  /* dpi */
};

gvplugin_installed_t gvdevice_gdk_types[] = {
    {FORMAT_BMP, "bmp:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_ICO, "ico:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_JPEG, "jpe:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_JPEG, "jpeg:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_JPEG, "jpg:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_PNG, "png:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_TIFF, "tif:cairo", 6, &gdk_engine, &device_features_gdk},
    {FORMAT_TIFF, "tiff:cairo", 6, &gdk_engine, &device_features_gdk},
    {0, NULL, 0, NULL, NULL}
};
