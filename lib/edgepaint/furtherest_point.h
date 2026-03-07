/*************************************************************************
 * Copyright (c) 2014 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

void furtherest_point(int k, int dim, double *wgt, double *pts, double *center, double width, int max_level, double *dist_max, double **argmax);
void furtherest_point_in_list(int k, int dim, double *wgt, double *pts, QuadTree qt, int max_level,
			      double *dist_max, double **argmax);
