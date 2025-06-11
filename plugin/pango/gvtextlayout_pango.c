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
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <gvc/gvplugin_render.h>
#include <common/utils.h>
#include <gvc/gvplugin_textlayout.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/gv_math.h>
#include <util/xml.h>

#include <pango/pangocairo.h>
#include "gvgetfontlist.h"
#ifdef HAVE_PANGO_FC_FONT_LOCK_FACE
#include <pango/pangofc-font.h>
#endif

static void pango_free_layout (void *layout)
{
    g_object_unref(layout);
}

static char* pango_psfontResolve (PostscriptAlias* pa)
{
    agxbuf buf = {0};
    agxbprint(&buf, "%s,", pa->family);
    if (pa->weight) {
        agxbprint(&buf, " %s", pa->weight);
    }
    if (pa->stretch) {
        agxbprint(&buf, " %s", pa->stretch);
    }
    if (pa->style) {
        agxbprint(&buf, " %s", pa->style);
    }
    return agxbdisown(&buf);
}

#define FONT_DPI 96.

#define ENABLE_PANGO_MARKUP

// wrapper to handle difference in calling conventions between `agxbput` and
// `gv_xml_escape`’s `cb`
static int agxbput_int(void *buffer, const char *s) {
  size_t len = agxbput(buffer, s);
  assert(len <= INT_MAX);
  return (int)len;
}

static bool pango_textlayout(textspan_t * span, char **fontpath)
{
    static agxbuf buf; // returned in fontpath, only good until next call
    static PangoFontMap *fontmap;
    static PangoContext *context;
    static PangoFontDescription *desc;
    static char *fontname;
    static double fontsize;
    static gv_font_map* gv_fmap;
    char *fnt, *psfnt = NULL;
    PangoFont *font;
#ifdef ENABLE_PANGO_MARKUP
    PangoAttrList *attrs;
    GError *error = NULL;
    int flags;
#endif
    bool text_needs_free = false;
    char *text;

    if (!context) {
	fontmap = pango_cairo_font_map_new();
	gv_fmap = get_font_mapping(fontmap);
	context = pango_font_map_create_context (fontmap);
	cairo_font_options_t* options = cairo_font_options_create();
	cairo_font_options_set_antialias(options,CAIRO_ANTIALIAS_GRAY);
	cairo_font_options_set_hint_style(options,CAIRO_HINT_STYLE_FULL);
	cairo_font_options_set_hint_metrics(options,CAIRO_HINT_METRICS_ON);
	cairo_font_options_set_subpixel_order(options,CAIRO_SUBPIXEL_ORDER_BGR);
	pango_cairo_context_set_font_options(context, options);
	pango_cairo_context_set_resolution(context, FONT_DPI);
	cairo_font_options_destroy(options);
	g_object_unref(fontmap);
    }

    if (!fontname || strcmp(fontname, span->font->name) != 0 ||
        !is_exactly_equal(fontsize, span->font->size)) {

	/* check if the conversion to Pango units below will overflow */
	if (INT_MAX / PANGO_SCALE < span->font->size) {
	    return false;
	}

	free(fontname);
	fontname = gv_strdup(span->font->name);
	fontsize = span->font->size;
	pango_font_description_free (desc);

	PostscriptAlias *pA = span->font->postscript_alias;
	bool psfnt_needs_free = false;
	if (pA) {
	    psfnt = fnt = gv_fmap[pA->xfig_code].gv_font;
	    if(!psfnt) {
		psfnt = fnt = pango_psfontResolve (pA);
		psfnt_needs_free = true;
	    }
	}
	else
	    fnt = fontname;

	desc = pango_font_description_from_string(fnt);
        // all text layout is done at a scale of FONT_DPI (nominally 96.)
        pango_font_description_set_size (desc, (int)(fontsize * PANGO_SCALE));

        if (fontpath && (font = pango_font_map_load_font(fontmap, context, desc))) {  /* -v support */
	    const char *fontclass = G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(font));

	    agxbclear(&buf);
	    if (psfnt) {
		agxbprint(&buf, "(ps:pango  %s) ", psfnt);
	    }
	    agxbprint(&buf, "(%s) ", fontclass);
#ifdef HAVE_PANGO_FC_FONT_LOCK_FACE
	    if (strcmp(fontclass, "PangoCairoFcFont") == 0) {
	        PangoFcFont *fcfont = PANGO_FC_FONT(font);
// `pango_fc_font_lock_face` arrived in Pango 1.4 and then was deprecated in
// 1.44. Its replacement is `pango_font_get_hb_font`. However this replacement
// does not seem to provide access to the underlying FreeType font information
// we want. Since this code path is only enabled when `pango_fc_font_lock_face`
// is available, suppress deprecation warnings and we will have to deal with the
// loss of this informational output altogether when Pango finally removes it.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	        FT_Face face = pango_fc_font_lock_face(fcfont);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
	        if (face) {
		    agxbprint(&buf, "\"%s, %s\" ", face->family_name, face->style_name);

		    FT_Stream stream = face->stream;
		    if (stream) {
			FT_StreamDesc streamdesc = stream->pathname;
			if (streamdesc.pointer)
			    agxbput(&buf, streamdesc.pointer);
		        else
			    agxbput(&buf, "*no pathname available*");
		    }
		    else
			agxbput(&buf, "*no stream available*");
		}
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	        pango_fc_font_unlock_face(fcfont);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
	    }
	    else
#endif
	    {
    		PangoFontDescription *tdesc = pango_font_describe(font);
		char *tfont = pango_font_description_to_string(tdesc);
	        agxbprint(&buf, "\"%s\" ", tfont);
	        g_free(tfont);
	    }
            *fontpath = agxbuse(&buf);
        }
        if (psfnt_needs_free) {
            free(psfnt);
        }
    }

#ifdef ENABLE_PANGO_MARKUP
    if (span->font && (flags = span->font->flags)) {
	agxbuf xb = {0};

	agxbput(&xb,"<span");

	if (flags & HTML_BF)
	    agxbput(&xb," weight=\"bold\"");
	if (flags & HTML_IF)
	    agxbput(&xb," style=\"italic\"");
	if (flags & HTML_UL)
	    agxbput(&xb," underline=\"single\"");
	if (flags & HTML_S)
	    agxbput(&xb," strikethrough=\"true\"");
	agxbput (&xb,">");

	if (flags & HTML_SUP)
	    agxbput(&xb,"<sup>");
	if (flags & HTML_SUB)
	    agxbput(&xb,"<sub>");

	const xml_flags_t xml_flags = {.raw = 1, .dash = 1, .nbsp = 1};
	gv_xml_escape(span->str, xml_flags, agxbput_int, &xb);

	if (flags & HTML_SUB)
	    agxbput(&xb,"</sub>");
	if (flags & HTML_SUP)
	    agxbput(&xb,"</sup>");

	agxbput (&xb,"</span>");
	if (!pango_parse_markup (agxbuse(&xb), -1, 0, &attrs, &text, NULL, &error)) {
	    fprintf (stderr, "Error - pango_parse_markup: %s\n", error->message);
	    text = span->str;
	    attrs = NULL;
	} else {
	    text_needs_free = true;
	}
	agxbfree (&xb);
    }
    else {
	text = span->str;
	attrs = NULL;
    }
#else
    text = span->str;
#endif

    PangoLayout *layout = pango_layout_new (context);
    span->layout = layout;    /* layout free with textspan - see labels.c */
    span->free_layout = pango_free_layout;    /* function for freeing pango layout */

    pango_layout_set_text (layout, text, -1);
    pango_layout_set_font_description (layout, desc);
#ifdef ENABLE_PANGO_MARKUP
    if (attrs)
	pango_layout_set_attributes (layout, attrs);
#endif

    PangoRectangle logical_rect;
    pango_layout_get_extents (layout, NULL, &logical_rect);

    /* if pango doesn't like the font then it sets width=0 but height = garbage */
    if (logical_rect.width == 0)
	logical_rect.height = 0;

    const double textlayout_scale = POINTS_PER_INCH / (FONT_DPI * PANGO_SCALE);
    span->size.x = logical_rect.width * textlayout_scale;
    span->size.y = logical_rect.height * textlayout_scale;

    /* The y offset from baseline to 0,0 of the bitmap representation */
    span->yoffset_layout = pango_layout_get_baseline (layout) * textlayout_scale;

    /* The distance below midline for y centering of text strings */
    span->yoffset_centerline = 0.05 * span->font->size;

    const bool rc = logical_rect.width != 0 || strcmp(text, "") == 0;
    if (text_needs_free) {
	g_free(text);
    }
    return rc;
}

static gvtextlayout_engine_t pango_textlayout_engine = {
    pango_textlayout,
};

gvplugin_installed_t gvtextlayout_pango_types[] = {
    {0, "textlayout", 10, &pango_textlayout_engine, NULL},
    {0, NULL, 0, NULL, NULL}
};
