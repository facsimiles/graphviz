/*************************************************************************
 * Copyright (c) 2014 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <stdbool.h>

enum { COLOR_RGB, COLOR_GRAY, COLOR_LAB };
enum { ERROR_BAD_COLOR_SCHEME = -9 };

/// for a graph A, get a distinctive color of its nodes so that the color
/// distance among all neighboring nodes are maximized
///
/// Here color distance on a node is defined as the minimum of color differences
/// between a node and its neighbors (or the minimum of weighted color
/// differences if weightedQ = true, where weights are stored as entries of A0.
/// Accuracy is the threshold given so that when finding the coloring for each
/// node, the optimal is with in "accuracy" of the true global optimal.
///
/// @param color_scheme rgb, gray, lab, or one of the color palettes in
///   color_palettes.h, or a list of hex rgb colors separated by comma like
///   "#ff0000,#00ff00"
/// @param lightness of the form 0,70, specifying the range of lightness of LAB
///   color. Ignored if scheme is not COLOR_LAB.
/// @param A the graph of n nodes
/// @param accuracy how accurate to find the optimal
/// @param seed random_seed. If negative, consider -seed as the number of random
///   start iterations
/// @param cdim dimension of the color space
/// @param color On input an array of size n*cdim, if NULL, will be allocated.
///   On exit the final color assignment for node i is [cdim*i,cdim*(i+1)), in
///   RGB (between 0 to 1)
int node_distinct_coloring(const char *color_scheme, int *lightness,
                           bool weightedQ, SparseMatrix A, double accuracy,
                           int seed, int *cdim, double **colors);
