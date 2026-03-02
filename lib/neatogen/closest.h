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

#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/defs.h>

PRIVATE void closest_pairs2graph(double *, int, int, vtx_data **);

#ifdef __cplusplus
}
#endif
