/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <glcomp/glcomplabel.h>
#include <glcomp/glcompfont.h>
#include <glcomp/glcompset.h>
#include <glcomp/glutils.h>
#include <util/alloc.h>

static void glCompLabelDraw(void *label) {
  glCompLabel *p = label;
  glCompCommon ref = p->base.common;
  glCompCalcWidget(p->base.common.parent, &p->base.common, &ref);
  glCompRenderText(p->base.common.font, &p->base);
}

glCompLabel *glCompLabelNew(void *par, char *text) {
    glCompLabel *p = gv_alloc(sizeof(glCompLabel));
    glCompInitCommon(&p->base, par, 0, 0);

    p->text = gv_strdup(text);
    p->base.common.font = glNewFontFromParent(&p->base, text);
    p->base.common.functions.draw = glCompLabelDraw;

    return p;
}
