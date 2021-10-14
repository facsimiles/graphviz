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

enum {COLOR_RGB, COLOR_GRAY, COLOR_LAB};
enum {ERROR_BAD_COLOR_SCHEME = -9};
int node_distinct_coloring(char *color_scheme, char *lightness, int weightedQ, SparseMatrix A, real accuracy, int iter_max, int seed, int *cdim, real **colors);
