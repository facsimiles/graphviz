/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <sparse/general.h>
#include <sparse/QuadTree.h>
#include <edgepaint/furtherest_point.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <util/alloc.h>
#include <util/debug.h>
#include <util/gv_math.h>
#include <util/list.h>
#include <util/prisize_t.h>

static double dist(int dim, double *x, double *y){
  int k;
  double d = 0;
  for (k = 0; k < dim; k++) d += (x[k] - y[k])*(x[k]-y[k]);
  return sqrt(d);
}


static double distance_to_group(int k, int dim, double *wgt, double *pts, double *center){
  int i;
  double distance = -1, dist_min = 0;
  if (!wgt){
    for (i = 0; i < k; i++){
      distance = dist(dim, &(pts[i*dim]), center);
      if (i == 0){
	dist_min = distance;
      } else {
	dist_min = MIN(dist_min, distance);
      }
    }
  } else {
    for (i = 0; i < k; i++){
      distance = dist(dim, &(pts[i*dim]), center);
      if (i == 0){
	dist_min = wgt[i]*distance;
      } else {
	dist_min = MIN(dist_min, wgt[i]*distance);
      }
    }
  }
  return dist_min;
}

typedef LIST(QuadTree) qt_list_t;

void furtherest_point(int k, int dim, double *wgt, double *pts, double *center, double width, int max_level, double *dist_max, double **argmax){
  /* Assume that in the box defined by {center, width} are feasible;     
     find, in the, a point "furtherest_point" that is furtherest away from a group of k-points pts, using quadtree,
     with up to max_level. Here the distance of a point to a group of point is defined as the minimum 
     of the distance of that point to all the points in the group,
     and each distance is defined by the function dist.

     Input:
     
     k: number of points in the group
     dim: dimension
     wgt: if not null, the weighting factor for the i-th point is wgt[i]. The color distance
     .    of a point p to a group of points pts is min_i(wgt[i]*dist(p, pts[i])), instead of min_i(dist(p, pts[i]))
     pts: the i-th point is [i*k, (i+1)*k)
     center: the center of the root of quadtree
     width: the width of the root
     max_level: max level to go down
     argmax: on entry, if NULL, will be allocated, iotherwise must be an array of size >= dim which will hold the furtherest point. 

     Return: the point (argmax) furtherest away from the group, and the distance dist_max.
   */
  QuadTree qt, qt0;
  double distance;
  int level = 0;
  int ii, j;
  double wmax = 0;

  if (wgt){
    for (int i = 0; i < k; i++) wmax = MAX(wgt[i], wmax);
  } else {
    wmax = 1.;
  }

  qt0 = qt = QuadTree_new(dim, center, width, max_level);

  qt->total_weight = *dist_max = distance_to_group(k, dim, wgt, pts, center);/* store distance in total_weight */
  if (!(*argmax)) *argmax = gv_calloc(dim, sizeof(double));
  memcpy(*argmax, center, sizeof(double)*dim);

  qt_list_t candidates = {0};
  qt_list_t candidates2 = {0};
  LIST_APPEND(&candidates, qt);

  /* idea: maintain the current best point and best (largest) distance. check the list of candidate. Subdivide each into quadrants, if any quadrant gives better distance, update, and put on the candidate
     list. If we can not prune a quadrant (a quadrant can be pruned if the distance of its center to the group of points pts, plus that from the center to the corner of the quadrant, is smaller than the best), we
     also put it down on the candidate list. We then recurse on the candidate list, unless the max level is reached. */
  while (level++ < max_level){ 
    if (Verbose > 10) {
      fprintf(stderr,"level=%d=================\n", level);
    }
    LIST_CLEAR(&candidates2);
    for (size_t i = 0; i < LIST_SIZE(&candidates); i++){


      qt = LIST_GET(&candidates, i);
      assert(!(qt->qts));

      if (Verbose > 10) {
	fprintf(stderr, "candidate %" PRISIZE_T " at {", i);
	for (j = 0; j < dim; j++) fprintf(stderr,"%f, ", qt->center[j]);
	fprintf(stderr,"}, width = %f, dist = %f\n", qt->width, qt->total_weight);
      }

      distance = qt->total_weight;/* total_weight is used to store the distance from the center to the group */
      if (distance + wmax*sqrt(((double) dim))*qt->width < *dist_max) continue;/* this could happen if this candidate was entered into the list earlier than a better one later in the list */
      qt->qts = gv_calloc((size_t)1 << dim, sizeof(QuadTree));
      for (ii = 0; ii < 1<<dim; ii++) {
	qt->qts[ii] = QuadTree_new_in_quadrant(qt->dim, qt->center, (qt->width)/2, max_level, ii);
	qt->qts[ii]->total_weight = distance = distance_to_group(k, dim, wgt, pts, qt->qts[ii]->center);/* store distance in total_weight */
	bool pruned = false;
	if (distance > *dist_max){
	  *dist_max = distance;
	  if (Verbose > 10) {
	    fprintf(stderr,"new distmax=%f, pt={", *dist_max);
	    for (j = 0; j < dim; j++) fprintf(stderr,"%f, ", qt->qts[ii]->center[j]);
	    fprintf(stderr,"}\n");
 	  }
	  memcpy(*argmax, qt->qts[ii]->center, sizeof(double)*dim);
	} else if (distance + wmax*sqrt(((double) dim))*(qt->width)/2 < *dist_max){
	  pruned = true;
	}
	if (!pruned){
	  LIST_APPEND(&candidates2, qt->qts[ii]);
	}
      }/* finish checking every of the 2^dim siblings */
    }/* finish checking all the candidates */

    SWAP(&candidates, &candidates2);

  }/* continue down the quadtree */

  QuadTree_delete(qt0);

  LIST_FREE(&candidates);
  LIST_FREE(&candidates2);
}

void furtherest_point_in_list(int k, int dim, double *wgt, double *pts, QuadTree qt, int max_level, 
			      double *dist_max, double **argmax){
  /* Given a list of points in the Euclidean space contained in the quadtree qt (called the feasible points), find among them one that
     is closest to another list of point {dim, k, pts}.


     find, in the, a point "furtherest_point" that is furtherest away from a group of k-points pts, using quadtree,
     with up to max_level. Here the distance of a point to a group of point is defined as the minimum 
     of the distance of that point to all the points in the group,
     and each distance is defined by the function dist.

     Input:
     
     k: number of points in the group
     dim: dimension
     wgt: if not null, the weighting factor for the i-th point is wgt[i]. The color distance
     .    of a point p to a group of points pts is min_i(wgt[i]*dist(p, pts[i])), instead of min_i(dist(p, pts[i]))
     pts: the i-th point is [i*k, (i+1)*k)
     center: the center of the root of quadtree
     width: the width of the root
     max_level: max level to go down
     argmax: on entry, if NULL, will be allocated, otherwise must be an array of size >= dim which will hold the furtherest point. 

     Return: the point (argmax) furtherest away from the group, and the distance dist_max.
   */

  double distance;
  int level = 0;
  int ii, j;
  double *average;
  double wmax = 0.;

  if (wgt){
    for (int i = 0; i < k; i++) wmax = MAX(wgt[i], wmax);
  } else {
    wmax = 1.;
  }

  average = qt->average;
  qt->total_weight = *dist_max = distance_to_group(k, dim, wgt, pts, average);/* store distance in total_weight */
  if (!(*argmax)) *argmax = gv_calloc(dim, sizeof(double));
  memcpy(*argmax, average, sizeof(double)*dim);

  qt_list_t candidates = {0};
  qt_list_t candidates2 = {0};
  LIST_APPEND(&candidates, qt);

  /* idea: maintain the current best point and best (largest) distance. check the list of candidate. Subdivide each into quadrants, if any quadrant gives better distance, update, and put on the candidate
     list. If we can not prune a quadrant (a quadrant can be pruned if the distance of its center to the group of points pts, plus that from the center to the corner of the quadrant, is smaller than the best), we
     also put it down on the candidate list. We then recurse on the candidate list, unless the max level is reached. */
  while (level++ < max_level){ 
    if (Verbose > 10) {
      fprintf(stderr,"level=%d=================\n", level);
    }
    LIST_CLEAR(&candidates2);
    for (size_t i = 0; i < LIST_SIZE(&candidates); i++){
      qt = LIST_GET(&candidates, i);

      if (Verbose > 10) {
	fprintf(stderr,"candidate %" PRISIZE_T " at {", i);
	for (j = 0; j < dim; j++) fprintf(stderr,"%f, ", qt->center[j]);
	fprintf(stderr,"}, width = %f, dist = %f\n", qt->width, qt->total_weight);
      }

      distance = qt->total_weight;/* total_weight is used to store the distance from average feasible points to the group */
      if (qt->n == 1 || distance + wmax*2*sqrt(((double) dim))*qt->width < *dist_max) continue;/* this could happen if this candidate was entered into the list earlier than a better one later in the list. Since the distance
									     is from the average of the feasible points in the square which may not be at the center */

      if (!(qt->qts)) continue;

      for (ii = 0; ii < 1<<dim; ii++) {
	if (!(qt->qts[ii])) continue;
	qt->qts[ii]->total_weight = distance = distance_to_group(k, dim, wgt, pts, qt->qts[ii]->average);/* store distance in total_weight */
	bool pruned = false;
	if (distance > *dist_max){
	  *dist_max = distance;
	  if (Verbose > 10) {
	    fprintf(stderr,"new distmax=%f, pt={", *dist_max);
	    for (j = 0; j < dim; j++) fprintf(stderr,"%f, ", qt->qts[ii]->average[j]);
	    fprintf(stderr,"}\n");
 	  }
	  memcpy(*argmax, qt->qts[ii]->average, sizeof(double)*dim);
	} else if (distance + wmax*sqrt(((double) dim))*(qt->width) < *dist_max){/* average feasible point in this square is too close to the point set */
	  pruned = true;
	}
	if (!pruned){
	  LIST_APPEND(&candidates2, qt->qts[ii]);
	}
      }/* finish checking every of the 2^dim siblings */
    }/* finish checking all the candidates */

    SWAP(&candidates, &candidates2);

  }/* continue down the quadtree */

  LIST_FREE(&candidates);
  LIST_FREE(&candidates2);
}
