/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

///////////////////////////////////////
//                                   // 
// This file contains the functions  //
// for distorting the layout.        //
//                                   //
// Four methods are available:       //
// 1) Uniform denisity - rectilinear //
// 2) Uniform denisity - polar       //
// 3) Fisheye - rectilinear          //
// 4) Fisheye - Ploar                //
//                                   // 
///////////////////////////////////////

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <neatogen/matrix_ops.h>
#include <neatogen/delaunay.h>
#include <common/arith.h>
#include <topfish/hierarchy.h>
#include <util/alloc.h>
#include <util/sort.h>

static double *compute_densities(v_data *graph, size_t n, double *x, double *y)
{
// compute density of every node by calculating the average edge length in a 2-D layout
    int j, neighbor;
    double sum;
    double *densities = gv_calloc(n, sizeof(double));

    for (size_t i = 0; i < n; i++) {
	sum = 0;
	for (j = 1; j < graph[i].nedges; j++) {
	    neighbor = graph[i].edges[j];
	    sum += hypot(x[i] - x[neighbor], y[i] - y[neighbor]);
	}
	densities[i] = sum / graph[i].nedges;
    }
    return densities;
}

static double *smooth_vec(double *vec, int *ordering, size_t n, int interval) {
// smooth 'vec' by setting each components to the average of is 'interval'-neighborhood in 'ordering'
    assert(interval >= 0);
	double sum;
    double *smoothed_vec = gv_calloc(n, sizeof(double));
    size_t n1 = MIN(1 + (size_t)interval, n);
    sum = 0;
    for (size_t i = 0; i < n1; i++) {
	sum += vec[ordering[i]];
    }

    size_t len = n1;
    for (size_t i = 0; i < MIN(n, (size_t)interval); i++) {
	smoothed_vec[ordering[i]] = sum / (double)len;
	if (len < n) {
	    sum += vec[ordering[len++]];
	}
    }
    if (n <= (size_t)interval) {
	return smoothed_vec;
    }

    for (size_t i = (size_t)interval; i < n - (size_t)interval - 1; i++) {
	smoothed_vec[ordering[i]] = sum / (double)len;
	sum +=
	    vec[ordering[i + (size_t)interval + 1]] - vec[ordering[i - (size_t)interval]];
    }
    for (size_t i = MAX(n - (size_t)interval - 1, (size_t)interval); i < n; i++) {
	smoothed_vec[ordering[i]] = sum / (double)len;
	sum -= vec[ordering[i - (size_t)interval]];
	len--;
    }
    return smoothed_vec;
}

static int cmp(const void *a, const void *b, void *context) {
  const int *x = a;
  const int *y = b;
  const double *place = context;

  if (place[*x] < place[*y]) {
    return -1;
  }
  if (place[*x] > place[*y]) {
    return 1;
  }
  return 0;
}

void quicksort_place(double *place, int *ordering, size_t size) {
  gv_sort(ordering, size, sizeof(ordering[0]), cmp, place);
}

#define DIST(x1, y1, x2, y2) hypot((x1) - (x2), (y1) - (y2))

static void rescale_layout_polarFocus(v_data *graph, size_t n, double *x_coords,
                                      double *y_coords, double x_focus,
                                      double y_focus, int interval,
                                      double distortion) {
    // Polar distortion - auxiliary function
    double *densities = NULL;
    double *distances = gv_calloc(n, sizeof(double));
    double *orig_distances = gv_calloc(n, sizeof(double));

    for (size_t i = 0; i < n; i++)
	{
		distances[i] = DIST(x_coords[i], y_coords[i], x_focus, y_focus);
    }
    assert(n <= INT_MAX);
    copy_vector((int)n, distances, orig_distances);

    int *ordering = gv_calloc(n, sizeof(int));
    for (size_t i = 0; i < n; i++)
	{
		ordering[i] = (int)i;
    }
    quicksort_place(distances, ordering, n);

    densities = compute_densities(graph, n, x_coords, y_coords);
    double *smoothed_densities = smooth_vec(densities, ordering, n, interval);

    // rescale distances
    if (distortion < 1.01 && distortion > 0.99) 
	{
		for (size_t i = 1; i < n; i++)
		{
			distances[ordering[i]] =	distances[ordering[i - 1]] + (orig_distances[ordering[i]] -
					      orig_distances[ordering
							     [i -
							      1]]) / smoothed_densities[ordering[i]];
		}
    } else 
	{
		// just to make milder behavior:
		const double factor = (signbit(distortion) ? -1 : 1) * sqrt(fabs(distortion));
		for (size_t i = 1; i < n; i++)
		{
			distances[ordering[i]] =
				distances[ordering[i - 1]] + (orig_distances[ordering[i]] -
					      orig_distances[ordering
							     [i -
							      1]]) /
			pow(smoothed_densities[ordering[i]], factor);
		}
    }

    // compute new coordinate:
    for (size_t i = 0; i < n; i++)
	{
		const double ratio =
		  orig_distances[i] > 0 ? distances[i] / orig_distances[i] : 0;
		x_coords[i] = x_focus + (x_coords[i] - x_focus) * ratio;
		y_coords[i] = y_focus + (y_coords[i] - y_focus) * ratio;
    }

    free(densities);
    free(smoothed_densities);
    free(distances);
    free(orig_distances);
    free(ordering);
}

void
rescale_layout_polar(double *x_coords, double *y_coords,
		     double *x_foci, double *y_foci, int num_foci,
		     size_t n, int interval, double width,
		     double height, double distortion) {
    // Polar distortion - main function
    double minX, maxX, minY, maxY;
    double aspect_ratio;
    v_data *graph;
    double scaleX;
    double scale_ratio;

    // compute original aspect ratio
    minX = maxX = x_coords[0];
    minY = maxY = y_coords[0];
    for (size_t i = 1; i < n; i++)
	{
		minX = fmin(minX, x_coords[i]);
		minY = fmin(minY, y_coords[i]);
		maxX = fmax(maxX, x_coords[i]);
		maxY = fmax(maxY, y_coords[i]);
    }
    aspect_ratio = (maxX - minX) / (maxY - minY);

    // construct mutual neighborhood graph
    assert(n <= INT_MAX);
    graph = UG_graph(x_coords, y_coords, (int)n);

    if (num_foci == 1) 
	{	// accelerate execution of most common case
		rescale_layout_polarFocus(graph, n, x_coords, y_coords, x_foci[0],
				  y_foci[0], interval, distortion);
    } else
	{
	// average-based rescale
	double *final_x_coords = gv_calloc(n, sizeof(double));
	double *final_y_coords = gv_calloc(n, sizeof(double));
	double *cp_x_coords = gv_calloc(n, sizeof(double));
	double *cp_y_coords = gv_calloc(n, sizeof(double));
	assert(n <= INT_MAX);
	for (int i = 0; i < num_foci; i++) {
	    copy_vector((int)n, x_coords, cp_x_coords);
	    copy_vector((int)n, y_coords, cp_y_coords);
	    rescale_layout_polarFocus(graph, n, cp_x_coords, cp_y_coords,
				      x_foci[i], y_foci[i], interval, distortion);
	    scadd(final_x_coords, (int)n - 1, 1.0 / num_foci, cp_x_coords);
	    scadd(final_y_coords, (int)n - 1, 1.0 / num_foci, cp_y_coords);
	}
	copy_vector((int)n, final_x_coords, x_coords);
	copy_vector((int)n, final_y_coords, y_coords);
	free(final_x_coords);
	free(final_y_coords);
	free(cp_x_coords);
	free(cp_y_coords);
    }
    free(graph[0].edges);
    free(graph);

    minX = maxX = x_coords[0];
    minY = maxY = y_coords[0];
    for (size_t i = 1; i < n; i++) {
	minX = fmin(minX, x_coords[i]);
	minY = fmin(minY, y_coords[i]);
	maxX = fmax(maxX, x_coords[i]);
	maxY = fmax(maxY, y_coords[i]);
    }

    // shift points:
    for (size_t i = 0; i < n; i++) {
	x_coords[i] -= minX;
	y_coords[i] -= minY;
    }

    // rescale x_coords to maintain aspect ratio:
    scaleX = aspect_ratio * (maxY - minY) / (maxX - minX);
    for (size_t i = 0; i < n; i++) {
	x_coords[i] *= scaleX;
    }


    // scale the layout to fit full drawing area:
    scale_ratio =
	MIN((width) / (aspect_ratio * (maxY - minY)),
	    (height) / (maxY - minY));
    for (size_t i = 0; i < n; i++) {
	x_coords[i] *= scale_ratio;
	y_coords[i] *= scale_ratio;
    }
}
