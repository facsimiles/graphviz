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

#include <cgraph/list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#include <tchar.h>
#endif
#include <glcomp/opengl.h>

#ifdef __cplusplus
extern "C" {
#endif


#define	GLCOMPSET_PANEL_COLOR_R		0.16f
#define	GLCOMPSET_PANEL_COLOR_G		0.44f
#define	GLCOMPSET_PANEL_COLOR_B		0.87f
#define	GLCOMPSET_PANEL_COLOR_ALPHA	0.5f
#define GLCOMPSET_PANEL_SHADOW_WIDTH		4.0f

#define	GLCOMPSET_BUTTON_COLOR_R		0.0f
#define	GLCOMPSET_BUTTON_COLOR_G		1.0f
#define	GLCOMPSET_BUTTON_COLOR_B		0.3f
#define	GLCOMPSET_BUTTON_COLOR_ALPHA	0.6f
#define	GLCOMPSET_BUTTON_THICKNESS		3.0f
#define	GLCOMPSET_BUTTON_BEVEL_BRIGHTNESS		1.7f
#define GLCOMPSET_FONT_SIZE				14.0f

#define	GLCOMPSET_FONT_COLOR_R		0.0f
#define	GLCOMPSET_FONT_COLOR_G		0.0f
#define	GLCOMPSET_FONT_COLOR_B		0.0f
#define	GLCOMPSET_FONT_COLOR_ALPHA	1.0f
#define GLCOMPSET_FONT_DESC  "Times Italic"

#define GLCOMPSET_BORDERWIDTH				2.0f
#define GLCOMPSET_PANEL_BORDERWIDTH				3.0f
#define GLCOMPSET_BUTTON_BEVEL				5.0f
#define	GLCOMPSET_BEVEL_DIFF				0.001f
#define	GLCOMP_DEFAULT_WIDTH	10.0f
#define	GLCOMP_DEFAULT_HEIGHT	10.0f

    typedef enum { glAlignNone, glAlignLeft, glAlignTop, glAlignBottom,
	    glAlignRight, glAlignParent, glAlignCenter } glCompAlignment;

    typedef enum { glFontVJustifyNone, glFontVJustifyCenter } glCompVJustify;
    typedef enum { glFontHJustifyNone, glFontHJustifyCenter } glCompHJustify;

    typedef enum { glMouseDown, glMouseUp } glCompMouseStatus;
    typedef enum { glMouseLeftButton, glMouseRightButton,
	    glMouseMiddleButton } glMouseButtonType;

    typedef enum { glTexImage, glTexLabel } glCompTexType;

    typedef struct glCompObj_ glCompObj;

/*call backs for widgets*/
    typedef void (*glcompdrawfunc_t) (void *obj);
    typedef void (*glcompclickfunc_t)(glCompObj *obj, float x,
				       float y, glMouseButtonType t);
    typedef void (*glcompdoubleclickfunc_t) (glCompObj * obj, float x,
					     float y,
					     glMouseButtonType t);
    typedef void (*glcompmouseoverfunc_t) (glCompObj * obj, float x,
					   float y);
    typedef void (*glcompmouseinfunc_t) (glCompObj * obj, float x,
					 float y);
    typedef void (*glcompmouseoutfunc_t) (glCompObj * obj, float x,
					  float y);
    typedef void (*glcompmousedownfunc_t) (glCompObj * obj, float x,
					   float y, glMouseButtonType t);
    typedef void (*glcompmouseupfunc_t) (glCompObj * obj, float x,
					 float y, glMouseButtonType t);
    typedef void (*glcompmousedragfunct_t) (glCompObj * obj, float dx,
					    float dy,
					    glMouseButtonType t);

    typedef struct {
	int topAnchor;		/*anchor booleans */
	int leftAnchor;
	int rightAnchor;
	int bottomAnchor;

	float top;		/*anchor values */
	float left;
	float right;
	float bottom;


    } glCompAnchor;

    typedef struct {
	glCompVJustify VJustify;
	glCompHJustify HJustify;
    } glCompJustify;

    typedef struct {
	float x, y, z;
    } glCompPoint;

DEFINE_LIST(glCompPoly, glCompPoint)

    typedef struct {
	float R;
	float G;
	float B;
	float A;		//Alpha
    } glCompColor;

    typedef struct {
	glCompPoint pos;
	float w;
	float h;
    } glCompRect;

    typedef struct {
	uint32_t id;
	char *def;
	char *text;
	int width;
	int height;
	glCompTexType type;
	int userCount;
	int fontSize;
	unsigned char *data;	/*data */
    } glCompTex;



/*opengl font*/
    typedef struct {
	char *fontdesc;		//font description , only used with pango fonts
	glCompColor color;
	void *glutfont;		/*glut font pointer if used */
	int transparent;
	glCompTex *tex;		/* texture, if type is pangotext */
	int size;
	glCompJustify justify;
	bool is2D;
    } glCompFont;

    typedef struct {
	glcompdrawfunc_t draw;
	glcompclickfunc_t click;
	glcompdoubleclickfunc_t doubleclick;
	glcompmouseoverfunc_t mouseover;
	glcompmouseinfunc_t mousein;
	glcompmouseoutfunc_t mouseout;
	glcompmousedownfunc_t mousedown;
	glcompmouseupfunc_t mouseup;
	glcompmousedragfunct_t mousedrag;

    } glCompCallBacks;

typedef struct glCompSet_ glCompSet;

/*
	common widget properties
	also each widget has pointer to its parents common
*/
    typedef struct {
	glCompPoint pos;
	glCompPoint refPos;	/*calculated pos after anchors and aligns */
	float width, height;
	float borderWidth;
	glCompColor color;
	int enabled;
	int visible;
	glCompSet *compset; ///< compset
	void *parent;		/*parent widget */
	int data;
	glCompFont font; ///< font to use
	glCompAlignment align;
	glCompAnchor anchor;
	int layer;		/*keep track of object order, what to draw on top */
	glCompCallBacks callbacks;
	glCompCallBacks functions;
	glCompJustify justify;
    } glCompCommon;

/// object prototype
struct glCompObj_ {
  glCompCommon common;
};

/*generic image*/
    typedef struct {
	glCompObj base;
	glCompTex *texture;
	float width, height;  /* width and height in world coords */
    } glCompImage;

/*generic panel*/
    typedef struct {
	glCompObj base;
	float shadowwidth;
	char *text;
	glCompImage *image;
    } glCompPanel;

/*label*/
    typedef struct {
	glCompObj base;
	char *text;
    } glCompLabel;

/*buttons*/
    typedef struct {
	glCompObj base;
	float width, height;
	glCompLabel *label;
	bool status; ///< false not pressed, true pressed
	bool refStatus; ///< false not pressed, true pressed
	int groupid;
	glCompImage *image;	/*glyph */
	int data;
    } glCompButton;

/*texture based image*/

    typedef struct {
	glMouseButtonType t;
	float x, y; ///< current mouse pos
	glCompPoint GLpos;/*3d converted opengl position*/
	glCompPoint GLinitPos;/*mouse button down pos*/
	glCompPoint GLfinalPos;/*mouse button up pos*/

	float dragX, dragY;/*GLpos - GLinitpos*/
	glCompObj *clickedObj;
	glCompCallBacks callbacks;
	glCompCallBacks functions;
	int down;


    } glCompMouse;



/*main widget set manager*/
    struct glCompSet_ {
	glCompObj base;
	glCompObj **obj;
	size_t objcnt;
	size_t textureCount;
	glCompTex **textures;
	glCompMouse mouse;
    };

#ifdef __cplusplus
}
#endif
