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

#include "smyrnadefs.h"

    extern glCompSet *glcreate_gl_topview_menu(void);
    extern void switch2D3D(glCompObj *obj, GLfloat x, GLfloat y, glMouseButtonType t);
    extern void menu_click_center(glCompObj *obj, GLfloat x, GLfloat y, glMouseButtonType t);
    extern void menu_click_zoom_minus(glCompObj *obj, GLfloat x, GLfloat y, glMouseButtonType t);
    extern void menu_click_zoom_plus(glCompObj *obj, GLfloat x, GLfloat y, glMouseButtonType t);
    extern void menu_click_pan(glCompObj *obj, GLfloat x, GLfloat y, glMouseButtonType t);
