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

#include <stdbool.h>
#include <stdlib.h>
#include <TargetConditionals.h>

#include <gvc/gvplugin_textlayout.h>
#include "gvplugin_quartz.h"

#if TARGET_OS_IPHONE
#include <CoreText/CoreText.h>
#endif

void *quartz_new_layout(char* fontname, double fontsize, char* text)
{
	CFStringRef fontnameref = CFStringCreateWithCString(NULL, fontname, kCFStringEncodingUTF8);
	CFStringRef textref = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
	CTLineRef line = NULL;
	
	if (fontnameref && textref) {
		/* set up the Core Text line */
		CTFontRef font = CTFontCreateWithName(fontnameref, fontsize, NULL);
		const void *attributeNames[] = {
		  kCTFontAttributeName, kCTForegroundColorFromContextAttributeName};
		const void *attributeValues[] = {font, kCFBooleanTrue};
		CFDictionaryRef attributes = CFDictionaryCreate(
			NULL,
			attributeNames,
			attributeValues,
			sizeof(attributeNames) / sizeof(attributeNames[0]),
			&kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
		CFAttributedStringRef attributed = CFAttributedStringCreate(NULL, textref, attributes);
		line = CTLineCreateWithAttributedString(attributed);
		
		CFRelease(attributed);
		CFRelease(attributes);
		CFRelease(font);
	}
	
	if (textref)
		CFRelease(textref);
	if (fontnameref)
		CFRelease(fontnameref);
// Suppress Clang/GCC -Wcast-qual warning. Casting away const here is acceptable
// as we do not intend to modify the target through the resulting non-const
// pointer. We need a non-const pointer simply to satisfy the type of
// `textspan_t.layout`.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	return (void *)line;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

void quartz_size_layout(void *layout, double* width, double* height, double* yoffset_layout)
{
	/* get the typographic bounds */
	CGFloat ascent = 0.0;
	CGFloat descent = 0.0;
	CGFloat leading = 0.0;
	
	*width = CTLineGetTypographicBounds(layout, &ascent, &descent, &leading);
	*height = ascent + descent + leading;
	*yoffset_layout = ascent;
}

void quartz_draw_layout(void *layout, CGContextRef context, CGPoint position)
{
	CGContextSetTextPosition(context, position.x, position.y);
	CTLineDraw(layout, context);
}

void quartz_free_layout(void *layout)
{
	if (layout)
		CFRelease(layout);
}

static bool quartz_textlayout(textspan_t *para, char **fontpath)
{
	(void)fontpath;

	void *line = quartz_new_layout(para->font->name, para->font->size, para->str);
	if (line)
	{
		/* report the layout */
		para->layout = line;
		para->free_layout = &quartz_free_layout;
		quartz_size_layout(line, &para->size.x, &para->size.y, &para->yoffset_layout);
		para->yoffset_centerline = 0.2 * para->font->size;
		return true;
	}
	else
		return false;
};

static gvtextlayout_engine_t quartz_textlayout_engine = {
    quartz_textlayout
};

gvplugin_installed_t gvtextlayout_quartz_types[] = {
    {0, "textlayout", 8, &quartz_textlayout_engine, NULL},
    {0, NULL, 0, NULL, NULL}
};
