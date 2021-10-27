/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <sparse/SparseMatrix.h>

typedef struct rectangle_struct {
  real x[2];/* center */
  real size[2]; /* total width/height*/
} rectangle;

extern rectangle* tree_map(int n, real *area, rectangle fillrec);

extern rectangle rectangle_new(real x, real y, real width, real height);
