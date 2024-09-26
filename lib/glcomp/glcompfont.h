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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <glcomp/glpangofont.h>
#endif
#include <glcomp/opengl.h>
#include <glcomp/glcompdefs.h>
#include <GL/glut.h>

#ifdef __cplusplus
extern "C" {
#endif

    void glprintfglut(void *font, float xpos, float ypos, float zpos, char *bf);

glCompFont glNewFont(glCompSet *s, char *text, glCompColor *c, char *fontdesc,
                     int fs, bool is2D);
glCompFont glNewFontFromParent(glCompObj *o, char *text);
    void glDeleteFont(glCompFont * f);
void glCompDrawText(glCompFont f, float x, float y);
void glCompRenderText(glCompFont f, glCompObj *parentObj);
void glCompDrawText3D(glCompFont f, float x, float y, double z, float w,
                      float h);

#ifdef __cplusplus
}
#endif
