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

#include <common/render.h>

#ifdef __cplusplus
extern "C" {
#endif

void osage_layout(Agraph_t *g);
void osage_cleanup(Agraph_t *g);

#ifdef __cplusplus
}
#endif
