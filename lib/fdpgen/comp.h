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

#include <stddef.h>
#include <util/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <common/render.h>

typedef LIST(graph_t *) graphs_t;

/// @param counter [in,out] State used for constructing distinct subgraph names
graphs_t findCComp(graph_t *, int *, size_t *counter);

#ifdef __cplusplus
}
#endif
