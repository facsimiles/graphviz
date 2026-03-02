/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <glcomp/glcompdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

glCompButton *glCompButtonNew(void *par, float x, float y, float w, float h,
                              char *caption);
    extern void glCompButtonDraw(glCompButton * p);
int glCompButtonAddPngGlyph(glCompButton *b, const char *fileName);
void glCompButtonClick(glCompObj *o, float x, float y, glMouseButtonType t);
void glCompButtonMouseDown(glCompObj *o, float x, float y,
				      glMouseButtonType t);
void glCompButtonMouseOver(glCompObj *o, float x, float y);
void glCompButtonMouseUp(glCompObj *o, float x, float y, glMouseButtonType t);
    extern void glCompButtonHide(glCompButton * p);
    extern void glCompButtonShow(glCompButton * p);

#ifdef __cplusplus
}
#endif
