/*************************************************************************
 * Copyright (c) 2014 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

void furtherest_point(int k, int dim, real *wgt, real *pts, real *center, real width, int max_level, real (*usr_dist)(int, real*, real*), real *dist_max, real **argmax);
void furtherest_point_in_list(int k, int dim, real *wgt, real *pts, QuadTree qt, int max_level,
			      real (*usr_dist)(int, real*, real*), real *dist_max, real **argmax);
