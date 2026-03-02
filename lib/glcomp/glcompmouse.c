/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <glcomp/glcompmouse.h>
#include <glcomp/glcompfont.h>
#include <glcomp/glcompset.h>
#include <glcomp/glutils.h>
#include <stdbool.h>

void glCompMouseInit(glCompMouse * m)
{
    m->functions.click = NULL;
    m->functions.draw = NULL;
    m->functions.mousedown = glCompMouseDown;
    m->functions.mouseover = glCompMouseOver;
    m->functions.mouseup = glCompMouseUp;

    m->callbacks.click = NULL;
    m->callbacks.mouseover = NULL;
    m->callbacks.mouseup = NULL;
    m->dragX = 0;
    m->dragY = 0;
    m->down = false;
}

void glCompMouseDown(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;
    (void)t;
}

void glCompMouseOver(glCompObj *obj, float x, float y) {
    (void)obj;
    (void)x;
    (void)y;
}

void glCompMouseUp(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;
    (void)t;
}
