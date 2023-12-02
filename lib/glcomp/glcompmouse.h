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

#ifdef __cplusplus
extern "C" {
#endif

/*events*/
    extern void glCompMouseInit(glCompMouse * m);
void glCompClick(glCompObj *o, float x, float y, glMouseButtonType t);
void glCompDoubleClick(glCompObj *obj, float x, float y, glMouseButtonType t);
void glCompMouseDown(glCompObj *obj, float x, float y, glMouseButtonType t);
void glCompMouseIn(glCompObj *obj, float x, float y);
void glCompMouseOut(glCompObj *obj, float x, float y);
void glCompMouseOver(glCompObj *obj, float x, float y);
void glCompMouseUp(glCompObj *obj, float x, float y, glMouseButtonType t);
void glCompMouseDrag(glCompObj *obj, float dx, float dy, glMouseButtonType t);

#ifdef __cplusplus
}
#endif
