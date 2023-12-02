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

/// color the edges of a graph so that conflicting edges are as distinct in
/// color as possible
///
/// @param color_scheme rgb, lab, gray, or a list of comma separated RGB colors
///   in hex, like #ff0000,#00ff00
/// @param lightness of the form 0,70, specifying the range of lightness of LAB
///   color. Ignored if scheme is not COLOR_LAB.
/// @param g the graph
/// @param angle if two edges cross at an angle < "angle", consider they as
///   conflict
/// @param accuracy how accurate when finding  color of an edge to be as
///   different from others
/// @param check_edges_with_same_endpoint if TRUE, we will check edges with same
///   end point and only consider them as conflict if their angle is very small.
///   Edges that share an end point and is close to 180 degree are not consider
///   conflict.
/// @param seed random_seed. If negative, consider -seed as the number of random
///   start iterations
Agraph_t *edge_distinct_coloring(const char *color_scheme, int *lightness,
                                 Agraph_t *g, double angle, double accuracy,
                                 int check_edges_with_same_endpoint, int seed);
