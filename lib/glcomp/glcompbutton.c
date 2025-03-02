/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <glcomp/glcompbutton.h>
#include <glcomp/glcomplabel.h>
#include <glcomp/glcompimage.h>
#include <glcomp/glcompfont.h>
#include <glcomp/glutils.h>
#include <glcomp/glcompset.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <GL/glut.h>
#include <util/alloc.h>

glCompButton *glCompButtonNew(void *par, float x, float y, float w, float h,
                              char *caption) {
    glCompButton *p = gv_alloc(sizeof(glCompButton));
    glCompInitCommon(&p->base, par, x, y);
    /*customize button color */
    p->base.common.color.R = GLCOMPSET_BUTTON_COLOR_R;
    p->base.common.color.G = GLCOMPSET_BUTTON_COLOR_G;
    p->base.common.color.B = GLCOMPSET_BUTTON_COLOR_B;
    p->base.common.color.A = GLCOMPSET_BUTTON_COLOR_ALPHA;

    p->base.common.borderWidth = GLCOMPSET_BUTTON_BEVEL;

    p->base.common.width = w;
    p->base.common.height = h;
    p->status = false; // false not pressed, true pressed
    p->groupid = 0;
    p->base.common.callbacks.click = NULL;
    /*set event functions */

    p->base.common.functions.draw = (glcompdrawfunc_t)glCompButtonDraw;

    p->base.common.functions.click = glCompButtonClick;
    p->base.common.functions.mousedown = glCompButtonMouseDown;
    p->base.common.functions.mouseover = glCompButtonMouseOver;
    p->base.common.functions.mouseup = glCompButtonMouseUp;

    /*caption */
    glDeleteFont(&p->base.common.font);
    p->base.common.font = glNewFontFromParent(&p->base, NULL);
    p->label = glCompLabelNew(p, caption);
    p->label->base.common.font.justify.VJustify = glFontVJustifyCenter;
    p->label->base.common.font.justify.HJustify = glFontHJustifyCenter;
    p->label->base.common.align = glAlignParent;
    /*image */
    p->image = NULL;
    return p;
}

int glCompButtonAddPngGlyph(glCompButton *b, const char *fileName) {
    int rv;
    /*delete if there is an existing image */
    if (b->image)
	glCompImageDelete(b->image);
    /*image on left for now */
    b->image = glCompImageNew(b, 0, 0);

    rv = glCompImageLoadPng(b->image, fileName);
    if (rv) {
	b->image->base.common.anchor.leftAnchor = 1;
	b->image->base.common.anchor.left = 0;

	b->image->base.common.anchor.topAnchor = 1;
	b->image->base.common.anchor.top = 0;

	b->image->base.common.anchor.bottomAnchor = 1;
	b->image->base.common.anchor.bottom = 0;

	b->label->base.common.anchor.leftAnchor = 1;
	b->label->base.common.anchor.left = b->image->base.common.width;
	b->label->base.common.anchor.rightAnchor = 1;
	b->label->base.common.anchor.right = 0;

	b->label->base.common.anchor.topAnchor = 1;
	b->label->base.common.anchor.top = 0;

	b->label->base.common.anchor.bottomAnchor = 1;
	b->label->base.common.anchor.bottom = 0;

	b->label->base.common.align = glAlignNone;
    }
    return rv;
}

void glCompButtonHide(glCompButton * p)
{
    p->base.common.visible = 0;
    if (p->label)
	p->label->base.common.visible = 0;
    if (p->image)
	p->image->base.common.visible = 0;
}

void glCompButtonShow(glCompButton * p)
{
    p->base.common.visible = 1;
    if (p->label)
	p->label->base.common.visible = 1;
    if (p->image)
	p->image->base.common.visible = 1;
}

void glCompButtonDraw(glCompButton * p)
{
    glCompCommon ref = p->base.common;
    glCompCalcWidget(p->base.common.parent, &p->base.common, &ref);
    if (!p->base.common.visible)
	return;
    /*draw panel */
    glCompDrawRectPrism(&(ref.pos), ref.width, ref.height,
                        p->base.common.borderWidth, 0.01f, &ref.color,
			!p->status);
    if (p->label)
	p->label->base.common.functions.draw(p->label);
    if (p->image)
	p->image->base.common.functions.draw(p->image);
}

void glCompButtonClick(glCompObj *o, float x, float y, glMouseButtonType t) {
    glCompButton *p = (glCompButton *) o;
    ((glCompButton *) o)->status=((glCompButton *) o)->refStatus ;
    if (p->groupid == -1) {
	p->status = !p->status;
    } else {
	p->status = false;
    }
    if (p->base.common.callbacks.click)
	p->base.common.callbacks.click(&p->base, x, y, t);
}

void glCompButtonMouseDown(glCompObj *obj, float x, float y,
			   glMouseButtonType t)
{
    (void)x;
    (void)y;
    (void)t;
    
    ((glCompButton *) obj)->refStatus = ((glCompButton *) obj)->status;
    ((glCompButton *) obj)->status = true;
}

void glCompButtonMouseOver(glCompObj *obj, float x, float y) {
    if (obj->common.callbacks.mouseover)
	obj->common.callbacks.mouseover(obj, x, y);
}

void glCompButtonMouseUp(glCompObj *obj, float x, float y, glMouseButtonType t)
{
    if (obj->common.callbacks.mouseup)
	obj->common.callbacks.mouseup(obj, x, y, t);
}
