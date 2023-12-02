/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <glcomp/glcompdefs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

glCompImage *glCompImageNewFile(float x, float y, const char *imgfile);
glCompImage *glCompImageNew(void *par, float x, float y);
    extern void glCompImageDelete(glCompImage * p);
int glCompImageLoad(glCompImage *i, const unsigned char *data, int width,
                    int height, bool is2D);
int glCompImageLoadPng(glCompImage *i, const char *pngFile);
    extern void glCompImageDraw(void *obj);

#ifdef __cplusplus
}
#endif
