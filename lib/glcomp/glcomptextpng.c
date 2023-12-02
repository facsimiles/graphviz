/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <glcomp/glcompfont.h>
#include <glcomp/glcomptextpng.h>
#include <gtk/gtk.h>
#include <png.h>
#include <stddef.h>

unsigned char *glCompLoadPng(cairo_surface_t **surface, const char *filename,
                             int *imageWidth, int *imageHeight) {
    *surface = cairo_image_surface_create_from_png(filename);
    if (*surface == NULL ||
        cairo_surface_status(*surface) != CAIRO_STATUS_SUCCESS) {
      return NULL;
    }
    *imageWidth = cairo_image_surface_get_width(*surface);
    *imageHeight = cairo_image_surface_get_height(*surface);
    return cairo_image_surface_get_data(*surface);
}
