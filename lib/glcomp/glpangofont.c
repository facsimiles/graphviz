/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <glcomp/glpangofont.h>
#include <stddef.h>

static PangoLayout *get_pango_layout(char *markup_text,
				     char *fontdescription, int fontsize,
				     double *width, double *height)
{
    PangoFontDescription *desc;
    PangoFontMap *fontmap;
    PangoContext *context;
    PangoLayout *layout;
    int pango_width, pango_height;
    char *text;
    PangoAttrList *attr_list;
    fontmap = pango_cairo_font_map_get_default();
    context = pango_font_map_create_context(fontmap);

    desc = pango_font_description_from_string(fontdescription);
    pango_font_description_set_size(desc, (int)(fontsize * PANGO_SCALE));

    if (!pango_parse_markup
	(markup_text, -1, '\0', &attr_list, &text, NULL, NULL))
	return NULL;
    layout = pango_layout_new(context);
    pango_layout_set_text(layout, text, -1);
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_attributes(layout, attr_list);
    pango_font_description_free(desc);
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

    pango_layout_get_size(layout, &pango_width, &pango_height);

    *width = (double) pango_width / PANGO_SCALE;
    *height = (double) pango_height / PANGO_SCALE;

    return layout;
}

unsigned char *glCompCreatePangoTexture(char *fontdescription, int fontsize,
                                        char *txt, cairo_surface_t **surface,
                                        int *w, int *h) {
    PangoLayout *layout;
    double width, height;
    *surface = NULL;

    layout =
	get_pango_layout(txt, fontdescription, fontsize, &width, &height);
    if (layout == NULL) {
        return NULL;
    }
    //create the right size canvas for character set
    *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width,
                                          (int) height);

    cairo_t *cr = cairo_create(*surface);
    //set pen color to white
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    //draw the text
    pango_cairo_show_layout(cr, layout);



    *w = (int) width;
    *h = (int) height;
    g_object_unref(layout);
    cairo_destroy(cr);

    return cairo_image_surface_get_data(*surface);
}
