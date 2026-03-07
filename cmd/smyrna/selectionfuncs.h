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

#include "draw.h"
#include <glcomp/opengl.h>

extern void pick_objects_rect(Agraph_t* g) ;
extern void deselect_all(Agraph_t* g);
extern void add_selpoly(Agraph_t *g, glCompPoly_t *selPoly, glCompPoint pt);
