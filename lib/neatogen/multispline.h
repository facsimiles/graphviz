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

#include <render.h>
#include <pathutil.h>
#include <util/api.h>

typedef struct router_s router_t;

PRIVATE void freeRouter (router_t* rtr);
PRIVATE router_t* mkRouter (Ppoly_t** obs, int npoly);
PRIVATE int makeMultiSpline(edge_t* e, router_t * rtr, int);
