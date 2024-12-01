/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <glcomp/glcompset.h>
#include <glcomp/glcomppanel.h>
#include <glcomp/glcomplabel.h>
#include <glcomp/glcompbutton.h>
#include <glcomp/glcompmouse.h>

#include <glcomp/glutils.h>
#include <stdlib.h>
#include <util/alloc.h>

static float startX, startY;

static int glCompPointInObject(glCompObj * p, float x, float y)
{
  return x > p->common.refPos.x
	    && x < p->common.refPos.x + p->common.width
	    && y > p->common.refPos.y
	    && y < p->common.refPos.y + p->common.height;
}

static glCompObj *glCompGetObjByMouse(glCompSet *s, glCompMouse *m) {
    glCompObj *rv = NULL;
    if (!s || !m)
	return NULL;
    for (size_t ind = 0; ind < s->objcnt; ind++) {
	if (s->obj[ind]->common.visible
	    && glCompPointInObject(s->obj[ind], m->x, m->y)) {
	    if (!rv || s->obj[ind]->common.layer >= rv->common.layer) {
		if (s->obj[ind]->common.functions.click)
		    rv = s->obj[ind];
	    }
	}
    }

    return rv;
}

static void glCompMouseMove(void *obj, float x, float y) {
  glCompSet *o = obj;
  o->mouse.x = x;
  o->mouse.y = o->base.common.height - y;
  o->mouse.dragY = o->mouse.y - startY;
  o->mouse.dragX = o->mouse.x - startX;
  if (o->base.common.callbacks.mouseover) {
    o->base.common.callbacks.mouseover(obj, x, y);
  }
}

static void glCompSetMouseClick(void *obj, float x, float y,
				glMouseButtonType t)
{
  glCompObj *o = obj;
  if (o->common.callbacks.click) {
    o->common.callbacks.click(obj, x, y, t);
  }
}

static void glCompSetMouseDown(void *obj, float x, float y,
			       glMouseButtonType t)
{
    glCompSet *o = obj;
    o->mouse.t = t;
    if (t == glMouseLeftButton) {
	o->mouse.x = x;
	o->mouse.y = o->base.common.height - y;
	o->mouse.clickedObj = glCompGetObjByMouse(o->base.common.compset,
	                                          &o->base.common.compset->mouse);
	if (o->mouse.clickedObj)
	    if (o->mouse.clickedObj->common.functions.mousedown)
		o->mouse.clickedObj->common.functions.mousedown(o->mouse.clickedObj, x, y, t);
    }
    o->mouse.down = 1;
    startX = x;
    startY = o->base.common.height - y;
    if (o->base.common.callbacks.mousedown)
	o->base.common.callbacks.mousedown(obj, x, y, t);
}

static void glCompSetMouseUp(void *obj, float x, float y, glMouseButtonType t) {
    float tempX = x;
    glCompSet *ob = obj;
    float tempY = ob->base.common.height - y;

    ob->mouse.down = 0;
    if (t == glMouseLeftButton) {
	glCompObj *o = NULL;
	glCompObj *o_clicked = ob->mouse.clickedObj;
	ob->mouse.x = tempX;
	ob->mouse.y = tempY;
	if (o_clicked)
	    o = glCompGetObjByMouse(obj, &ob->mouse);
	if (!o)
	    return;
	if (o == o_clicked)
	    o->common.functions.click(o, x, y, t);
    }
    if (ob->base.common.callbacks.mouseup)
	ob->base.common.callbacks.mouseup(obj, x, y, t);
    /*check if mouse is clicked or dragged */
    if (startX == (int)tempX && startY == tempY)
	glCompSetMouseClick(obj, x, y, t);
}

void glCompInitCommon(glCompObj *childObj, glCompObj *parentObj, float x,
                      float y) {
    glCompCommon *c;
    glCompCommon *parent;
    c = &childObj->common;
    c->align = glAlignNone;
    c->anchor.bottom = 0;
    c->anchor.left = 0;
    c->anchor.top = 0;
    c->anchor.right = 0;
    c->anchor.leftAnchor = 0;
    c->anchor.rightAnchor = 0;
    c->anchor.topAnchor = 0;
    c->anchor.bottomAnchor = 0;
    c->data = 0;
    c->enabled = 1;
    c->height = GLCOMP_DEFAULT_HEIGHT;;
    c->width = GLCOMP_DEFAULT_WIDTH;
    c->visible = 1;
    c->pos.x = x;
    c->pos.y = y;
    c->borderWidth = GLCOMPSET_BORDERWIDTH;

    /*NULL function pointers */
    childObj->common.callbacks.click = NULL;
    childObj->common.callbacks.doubleclick = NULL;
    childObj->common.callbacks.draw = NULL;
    childObj->common.callbacks.mousedown = NULL;
    childObj->common.callbacks.mousein = NULL;
    childObj->common.callbacks.mouseout = NULL;
    childObj->common.callbacks.mouseover = NULL;
    childObj->common.callbacks.mouseup = NULL;

    childObj->common.functions.click = NULL;
    childObj->common.functions.doubleclick = NULL;
    childObj->common.functions.draw = NULL;
    childObj->common.functions.mousedown = NULL;
    childObj->common.functions.mousein = NULL;
    childObj->common.functions.mouseout = NULL;
    childObj->common.functions.mouseover = NULL;
    childObj->common.functions.mouseup = NULL;



    if (parentObj) {
	c->parent = &parentObj->common;
	parent = &parentObj->common;
	c->color = parent->color;
	c->layer = parent->layer + 1;
	c->pos.z = parent->pos.z;
	glCompSetAddObj(parent->compset, childObj);
    } else {
	c->parent = NULL;
	c->color.R = GLCOMPSET_PANEL_COLOR_R;
	c->color.G = GLCOMPSET_PANEL_COLOR_G;
	c->color.B = GLCOMPSET_PANEL_COLOR_B;
	c->color.A = GLCOMPSET_PANEL_COLOR_ALPHA;
	c->layer = 0;
	c->pos.z = 0;
    }
    c->font = glNewFontFromParent(childObj, NULL);
}

void glCompEmptyCommon(glCompCommon * c)
{
  glDeleteFont(&c->font);
}

glCompSet *glCompSetNew(int w, int h)
{
    glCompSet *s = gv_alloc(sizeof(glCompSet));
    glCompInitCommon(&s->base, NULL, 0.0f, 0.0f);
    s->base.common.width = (float)w;
    s->base.common.height = (float)h;
    s->objcnt = 0;
    s->obj = NULL;
    s->textureCount = 0;
    s->textures = NULL;
    glDeleteFont(&s->base.common.font);
    s->base.common.font = glNewFontFromParent(&s->base, NULL);
    s->base.common.compset = s;
    s->base.common.functions.mouseover = (glcompmouseoverfunc_t)glCompMouseMove;
    s->base.common.functions.mousedown = (glcompmousedownfunc_t)glCompSetMouseDown;
    s->base.common.functions.mouseup = (glcompmouseupfunc_t)glCompSetMouseUp;
    glCompMouseInit(&s->mouse);
    return s;
}

void glCompSetAddObj(glCompSet * s, glCompObj * obj)
{
    s->obj = gv_recalloc(s->obj, s->objcnt, s->objcnt + 1, sizeof(glCompObj*));
    s->objcnt++;
    s->obj[s->objcnt - 1] = obj;
    obj->common.compset = s;
}

void glCompDrawBegin(void)	//pushes a gl stack 
{
    int vPort[4];

    glGetIntegerv(GL_VIEWPORT, vPort);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();


    glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);

    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

void glCompDrawEnd(void)	//pops the gl stack 
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

void glCompSetDraw(glCompSet *s) {
    glCompDrawBegin();
    for (size_t ind = 0; ind < s->objcnt; ind++) {
	s->obj[ind]->common.functions.draw(s->obj[ind]);
    }
    glCompDrawEnd();
}

void glcompsetUpdateBorder(glCompSet * s, int w, int h)
{
    if (w > 0 && h > 0) {
	s->base.common.width = (float)w;
	s->base.common.height = (float)h;
    }
}
