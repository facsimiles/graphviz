/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <Availability.h>
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#else
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	FORMAT_NONE,
	FORMAT_CGIMAGE,
	FORMAT_BMP,
	FORMAT_EXR,
	FORMAT_GIF,
	FORMAT_ICNS,
	FORMAT_ICO,
	FORMAT_JPEG,
	FORMAT_JPEG2000,
	FORMAT_PDF,
	FORMAT_PICT,
	FORMAT_PNG,
	FORMAT_PSD,
	FORMAT_SGI,
	FORMAT_TIFF,
	FORMAT_TGA
} format_type;

static const int BITS_PER_COMPONENT = 8;	/* bits per color component */

CFStringRef format_to_uti(format_type format);

extern CGDataConsumerCallbacks device_data_consumer_callbacks;

/* gvtextlayout_quartz.c in Mac OS X: layout is a CoreText CTLineRef */

void *quartz_new_layout(char* fontname, double fontsize, char* text);
void quartz_size_layout(void *layout, double* width, double* height, double* yoffset_layout);
void quartz_draw_layout(void *layout, CGContextRef context, CGPoint position);
void quartz_free_layout(void *layout);

#ifdef __cplusplus
}
#endif
