/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "glcompui.h"
#include <glcomp/glcompbutton.h>
#include <glcomp/glcomppanel.h>
#include <glcomp/glcomplabel.h>
#include <glcomp/glcompimage.h>
#include "gltemplate.h"
#include <glcomp/glutils.h>
#include "glmotion.h"
#include "topfisheyeview.h"
#include "toolboxcallbacks.h"
#include "viewportcamera.h"
#include "selectionfuncs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "frmobjectui.h"

static glCompPanel *sel = NULL;
static glCompButton *to3DBtn;
static glCompButton *to2DBtn;
static glCompButton *toFisheye;
static glCompButton *toNormal;
static glCompImage *imgFisheye;
static glCompImage *img3D;
static glCompButton *panBtn;


void menu_click_pan(glCompObj *obj, float x, float y, glMouseButtonType t) {
        (void)obj;
        (void)x;
        (void)y;
        (void)t;

        deselect_all(view->g[view->activeGraph]);
}

void menu_click_zoom_minus(glCompObj *obj, float x, float y,
				  glMouseButtonType t)
{
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    glmotion_zoom_inc(0);
}

void menu_click_zoom_plus(glCompObj *obj, float x, float y, glMouseButtonType t)
{
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    glmotion_zoom_inc(1);
}

static void menu_switch_to_fisheye(glCompObj *obj, float x, float y,
				   glMouseButtonType t)
{
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    if (!view->Topview->fisheyeParams.active)
	{
	    if (!view->Topview->fisheyeParams.h) {
		prepare_topological_fisheye(view->g[view->activeGraph],view->Topview);
	    g_timer_start(view->timer);
	}
	view->Topview->fisheyeParams.active = 1;
	glCompButtonShow(toNormal);
	glCompButtonHide(toFisheye);
	imgFisheye->base.common.visible = 1;


    } else {
	view->Topview->fisheyeParams.active = 0;
	g_timer_stop(view->timer);
	glCompButtonHide(toNormal);
	glCompButtonShow(toFisheye);
	imgFisheye->base.common.visible = 0;


    }
}

void menu_click_center(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    if (view->active_camera == SIZE_MAX) { // 2D mode
	btnToolZoomFit_clicked(NULL, NULL);
    } else {			/*there is active camera , adjust it to look at the center */

	view->cameras[view->active_camera]->targetx = 0;
	view->cameras[view->active_camera]->targety = 0;
	view->cameras[view->active_camera]->r = 20;

    }
}

void switch2D3D(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;

    if (t == glMouseLeftButton) {

	if (view->active_camera == SIZE_MAX) {

	    if (view->camera_count == 0) {
		menu_click_add_camera();
	    } else {
		view->active_camera = 0;	/*set to camera */
	    }
	    glCompButtonShow(to2DBtn);
	    glCompButtonHide(to3DBtn);
	    img3D->base.common.visible = 1;
	} else {		/*switch to 2d */

	    view->active_camera = SIZE_MAX; // set to camera
	    glCompButtonShow(to3DBtn);
	    glCompButtonHide(to2DBtn);
	    panBtn->base.common.callbacks.click(&panBtn->base, 0.0f, 0.0f, 0);
	    img3D->base.common.visible = 0;


	}
    }
}

static void CBglCompMouseUp(glCompObj *obj, float x, float y, glMouseButtonType t)
{
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    sel->base.common.visible = 0;
    sel->base.common.pos.x = -5000;
}

static void CBglCompMouseRightClick(glCompObj *obj, float x, float y,
			     glMouseButtonType t)
{
    (void)obj;

    if (t == glMouseRightButton) 
	{
		float X, Y, Z = 0;
		to3D((int) x, (int) y, &X, &Y, &Z);
    }
}

static void attrList(glCompObj *obj, float x, float y, glMouseButtonType t) {
	(void)obj;
	(void)x;
	(void)y;
	(void)t;

	showAttrsWidget();
}

static void glCompMouseMove(glCompObj *obj, float x, float y) {
    (void)x;
    (void)y;

    glCompMouse *m = &((glCompSet *) obj)->mouse;

    sel->base.common.visible = 1;

    if ((m->down) && (m->t == glMouseRightButton)) 
    {
	sel->base.common.pos.x = m->x - m->dragX;
	sel->base.common.pos.y = m->y - m->dragY;
	sel->base.common.width = m->dragX;
	sel->base.common.height = m->dragY;
	glexpose();
    }
}

static void selectedges(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    view->Topview->sel.selectEdges = !view->Topview->sel.selectEdges;
}

static void selectnodes(glCompObj *obj, float x, float y, glMouseButtonType t) {
    (void)obj;
    (void)x;
    (void)y;
    (void)t;

    view->Topview->sel.selectNodes = !view->Topview->sel.selectNodes;
}

glCompSet *glcreate_gl_topview_menu(void)
{
    float y = 5;
    float off = 43;
    glCompSet *s = glCompSetNew(view->w, view->h);
    glCompPanel *p = NULL;
    glCompButton *b = NULL;
    glCompImage *i = NULL;
    glCompColor c;
    s->base.common.callbacks.click = CBglCompMouseRightClick;

    p = glCompPanelNew(s, 25, 25, 45, 47);
    p->base.common.align = glAlignLeft;
    p->base.common.data = 0;

    /*pan */
    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *pan = smyrnaPath("pan.png");
      glCompButtonAddPngGlyph(b, pan);
      free(pan);
    }
    b->base.common.callbacks.click = menu_click_pan;
    panBtn = b;

    y = y + off;

    /*switch to fisheye */
    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *fisheye = smyrnaPath("fisheye.png");
      glCompButtonAddPngGlyph(b, fisheye);
      free(fisheye);
    }
    b->base.common.callbacks.click = menu_switch_to_fisheye;
    toFisheye = b;


    /*switch to normal mode */
    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *fisheye = smyrnaPath("no_fisheye.png");
      glCompButtonAddPngGlyph(b, fisheye);
      free(fisheye);
    }
    b->base.common.callbacks.click = menu_switch_to_fisheye;
    b->base.common.visible = 0;
    toNormal = b;

    y=y+off;
    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *threed = smyrnaPath("3D.png");
      glCompButtonAddPngGlyph(b, threed);
      free(threed);
    }
    b->base.common.callbacks.click = switch2D3D;
    to3DBtn = b;

    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *twod = smyrnaPath("2D.png");
      glCompButtonAddPngGlyph(b, twod);
      free(twod);
    }
    b->base.common.callbacks.click = switch2D3D;
    glCompButtonHide(b);
    to2DBtn = b;

    y=y+off;
    b = glCompButtonNew(p, 1, y, 42, 42, "N");
    b->base.common.callbacks.click = selectnodes;
    b->groupid=-1;
    b->status = true;

    y=y+off;
    b = glCompButtonNew(p, 1, y, 42, 42, "E");
    b->base.common.callbacks.click = selectedges;
    b->groupid=-1;

    p = glCompPanelNew(p, 1, 325, 45, 180);
    p->base.common.align = glAlignTop;
    p->base.common.data = 0;
    p->base.common.borderWidth = 1;
    p->shadowwidth = 0;

    c.R = 0.80f;
    c.G = 0.6f;
    c.B = 0.6f;
    c.A = 1.6f;

    y = 1;

    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *details = smyrnaPath("details.png");
      glCompButtonAddPngGlyph(b, details);
      free(details);
    }
    b->base.common.callbacks.click = attrList;
    b->base.common.color = c;
	
    y = y + off;
	
	b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *zoomin = smyrnaPath("zoomin.png");
      glCompButtonAddPngGlyph(b, zoomin);
      free(zoomin);
    }
    b->groupid = 0;
    b->base.common.callbacks.click = menu_click_zoom_plus;
    b->base.common.color = c;
    y = y + off;

    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *zoomout = smyrnaPath("zoomout.png");
      glCompButtonAddPngGlyph(b, zoomout);
      free(zoomout);
    }
    b->base.common.callbacks.click = menu_click_zoom_minus;
    b->base.common.color = c;

    y = y + off;

    b = glCompButtonNew(p, 1, y, 42, 42, "");
    {
      char *center = smyrnaPath("center.png");
      glCompButtonAddPngGlyph(b, center);
      free(center);
    }
    b->base.common.callbacks.click = menu_click_center;
    b->base.common.color = c;

    p = glCompPanelNew(s, -250, 550, 150, 175);
    p->base.common.borderWidth = 0;
    p->shadowwidth = 0;
    p->base.common.color.R = 0;
    p->base.common.color.G = 0;
    p->base.common.color.B = 1;
    p->base.common.color.A = 0.2f;
    p->base.common.visible = 0;
    sel = p;
    s->base.common.callbacks.mouseover = glCompMouseMove;
    s->base.common.callbacks.mouseup = CBglCompMouseUp;

    p = glCompPanelNew(s, 25, 25, 52, 47);
    p->base.common.align = glAlignRight;
    p->base.common.data = 0;
    p->base.common.color.A = 0;

    p = glCompPanelNew(p, 25, 0, 52, 110);
    p->base.common.align = glAlignTop;
    p->base.common.data = 0;
    p->base.common.color.A = 0;
    p->shadowwidth = 0;

    i = glCompImageNew(p, 0, 0);
    {
      char *fisheye = smyrnaPath("mod_fisheye.png");
      glCompImageLoadPng(i, fisheye);
      free(fisheye);
    }
    imgFisheye = i;
    i->base.common.visible = 0;

    i = glCompImageNew(p, 0, 52);
    {
      char *threed = smyrnaPath("mod_3D.png");
      glCompImageLoadPng(i, threed);
      free(threed);
    }
    img3D = i;
    i->base.common.visible = 0;

    return s;



}
