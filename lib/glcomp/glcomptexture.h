/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*Open GL texture handling and storing mechanism
  includes glPanel,glCompButton,glCompCustomButton,clCompLabel,glCompStyle
*/

#pragma once

#ifdef _WIN32
#include "windows.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <glcomp/opengl.h>
#include <glcomp/glcompdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

glCompTex *glCompSetAddNewTexImage(glCompSet *s, int width, int height,
                                   const unsigned char *data, bool is2D);
glCompTex *glCompSetAddNewTexLabel(glCompSet *s, char *def, int fs,
                                   const char *text, bool is2D);

    extern void glCompDeleteTexture(glCompTex * t);
#ifdef __cplusplus
}
#endif
