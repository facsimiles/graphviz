/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <sparse/general.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sparse/SparseMatrix.h>
#include <edgepaint/edge_distinct_coloring.h>
#include <edgepaint/node_distinct_coloring.h>
#include <sparse/DotIO.h>
#include <edgepaint/intersection.h>
#include <sparse/QuadTree.h>
#include <util/alloc.h>

static int splines_intersect(size_t dim,
			     double cos_critical, int check_edges_with_same_endpoint, 
			     char *xsplines1, char *xsplines2){
  /* cos_critical: cos of critical angle
     check_edges_with_same_endpoint: whether need to treat two splines from
     .     the same end point specially in ignoring splines that exit/enter the same end pont at around 180
     xsplines1,xsplines2: the first and second splines corresponding to two edges

  */
  size_t len1 = 100, len2 = 100;
  size_t ns1 = 0, ns2 = 0;
  int iter1 = 0, iter2 = 0;
  double cos_a, tmp[2];
  int endp1 = 0, endp2 = 0;

  tmp[0] = tmp[1] = 0;
  double *x1 = gv_calloc(len1, sizeof(double));
  double *x2 = gv_calloc(len2, sizeof(double));

  assert(dim <= 3);

  /* splines could be a list of 
     1. 3n points
     2. of the form "e,x,y" followed by 3n points, where x,y is really padded to the end of the 3n points
     3. of the form "s,x,y" followed by 3n points, where x,y is padded to the start of the 3n points
  */
  if (xsplines1){
    if(strstr(xsplines1, "e,")){
      endp1 = 1;
      xsplines1 = strstr(xsplines1, "e,") + 2;
    } else if (strstr(xsplines1, "s,")){
      xsplines1 = strstr(xsplines1, "s,") + 2;
    }
  }
  while (xsplines1 && sscanf(xsplines1,"%lf,%lf", &(x1[ns1*dim]), &x1[ns1*dim + 1]) == 2){
    if (endp1 && iter1 == 0){
      tmp[0] = x1[ns1*dim]; tmp[1] = x1[ns1*dim + 1];
    } else {
      ns1++;
    }
    iter1++;
    xsplines1 = strchr(xsplines1, ' ');
    if (!xsplines1) break;
    xsplines1++;
    if (ns1*dim >= len1){
      size_t new_len1 = ns1 * dim + MAX(10u, ns1 * dim / 5);
      x1 = gv_recalloc(x1, len1, new_len1, sizeof(double));
      len1 = new_len1;
    }
  }
  if (endp1){/* pad the end point at the last position */
    ns1++;
    if (ns1*dim >= len1){
      size_t new_len1 = ns1 * dim + MAX(10u, ns1 * dim / 5);
      x1 = gv_recalloc(x1, len1, new_len1, sizeof(double));
      len1 = new_len1;
    }
    x1[(ns1-1)*dim] = tmp[0];  x1[(ns1-1)*dim + 1] = tmp[1]; 
  }


  /* splines could be a list of 
     1. 3n points
     2. of the form "e,x,y" followed by 3n points, where x,y is really padded to the end of the 3n points
     3. of the form "s,x,y" followed by 3n points, where x,y is padded to the start of the 3n points
  */
  if (xsplines2){
    if(strstr(xsplines2, "e,")){
      endp2 = 1;
      xsplines2 = strstr(xsplines2, "e,") + 2;
    } else if (strstr(xsplines2, "s,")){
      xsplines2 = strstr(xsplines2, "s,") + 2;
    }
  }
  while (xsplines2 && sscanf(xsplines2,"%lf,%lf", &(x2[ns2*dim]), &x2[ns2*dim + 1]) == 2){
    if (endp2 && iter2 == 0){
      tmp[0] = x2[ns2*dim]; tmp[1] = x2[ns2*dim + 1];
    } else {
      ns2++;
    }
    iter2++;
    xsplines2 = strchr(xsplines2, ' ');
    if (!xsplines2) break;
    xsplines2++;
    if (ns2*dim >= len2){
      size_t new_len2 = ns2 * dim + MAX(10u, ns2 * dim / 5);
      x2 = gv_recalloc(x2, len2, new_len2, sizeof(double));
      len2 = new_len2;
    }
  }
  if (endp2){/* pad the end point at the last position */
    ns2++;
    if (ns2*dim >= len2){
      size_t new_len2 = ns2 * dim + MAX(10u, ns2 * dim / 5);
      x2 = gv_recalloc(x2, len2, new_len2, sizeof(double));
      len2 = new_len2;
    }
    x2[(ns2-1)*dim] = tmp[0];  x2[(ns2-1)*dim + 1] = tmp[1]; 
  }

  for (size_t i = 0; i < ns1 - 1; i++) {
    for (size_t j = 0; j < ns2 - 1; j++) {
      cos_a = intersection_angle(&(x1[dim*i]), &(x1[dim*(i + 1)]), &(x2[dim*j]), &(x2[dim*(j+1)]));
      if (!check_edges_with_same_endpoint && cos_a >= -1) cos_a = fabs(cos_a);
      if (cos_a > cos_critical) {
	free(x1);
	free(x2);
	return 1;
      }

    }
  }

  free(x1);
  free(x2);
  return 0;
}

Agraph_t *edge_distinct_coloring(const char *color_scheme, int *lightness,
                                 Agraph_t *g, double angle, double accuracy,
                                 int check_edges_with_same_endpoint, int seed) {
  double *x = NULL;
  int dim = 2;
  SparseMatrix A, B, C;
  int *irn, *jcn, nz, nz2 = 0;
  double cos_critical = cos(angle/180*3.14159), cos_a;
  int u1, v1, u2, v2, i, j;
  double *colors = NULL;
  int flag, ne;
  char **xsplines = NULL;
  int cdim;

  A = SparseMatrix_import_dot(g, dim, &x, FORMAT_COORD);
  if (!x){
    fprintf(stderr,"The gv file contains no or improper 2D coordinates\n");
    return NULL;
  }


  irn = A->ia; jcn = A->ja;
  nz = A->nz;

  /* get rid of self edges */
  for (i = 0; i < nz; i++){
    if (irn[i] != jcn[i]){
      irn[nz2] = irn[i];
      jcn[nz2++] = jcn[i];
    }
  }

  if (Verbose)
    fprintf(stderr,"cos = %f, nz2 = %d\n", cos_critical, nz2);
  /* now find edge collision */
  B = SparseMatrix_new(nz2, nz2, 1, MATRIX_TYPE_REAL, FORMAT_COORD);

  if (Import_dot_splines(g, &ne, &xsplines)){
#ifdef TIME
    clock_t start = clock();
#endif
    assert(ne == nz2);
    cos_a = 1.;/* for splines we exit conflict check as soon as we find an conflict, so the anle may not be representitive, hence set to constant */
    for (i = 0; i < nz2; i++){
      for (j = i+1; j < nz2; j++){
	if (splines_intersect((size_t)dim, cos_critical,
	                      check_edges_with_same_endpoint, xsplines[i],
	                      xsplines[j])) {
	  B = SparseMatrix_coordinate_form_add_entry(B, i, j, &cos_a);
	}
      }
    }
#ifdef TIME
    fprintf(stderr, "cpu for dual graph =%10.3f", ((double) (clock() - start))/CLOCKS_PER_SEC);
#endif
    
  } else {
    /* no splines, justsimple edges */
#ifdef TIME
    clock_t start = clock();
#endif
    
    
    for (i = 0; i < nz2; i++){
      u1 = irn[i]; v1 = jcn[i];
      for (j = i+1; j < nz2; j++){
	u2 = irn[j]; v2 = jcn[j];
	cos_a = intersection_angle(&(x[dim*u1]), &(x[dim*v1]), &(x[dim*u2]), &(x[dim*v2]));
	if (!check_edges_with_same_endpoint && cos_a >= -1) cos_a = fabs(cos_a);
	if (cos_a > cos_critical) {
	  B = SparseMatrix_coordinate_form_add_entry(B, i, j, &cos_a);
	}
      }
    }
#ifdef TIME
    fprintf(stderr, "cpu for dual graph (splines) =%10.3f\n", ((double) (clock() - start))/CLOCKS_PER_SEC);
#endif
  } 
  C = SparseMatrix_from_coordinate_format(B);
  if (B != C) SparseMatrix_delete(B);
  
  {
#ifdef TIME
    clock_t start = clock();
#endif
    const bool weightedQ = false;
    flag = node_distinct_coloring(color_scheme, lightness, weightedQ, C,
                                  accuracy, seed, &cdim, &colors);
    if (flag) goto RETURN;
#ifdef TIME
    fprintf(stderr, "cpu for color assignmment =%10.3f\n", ((double) (clock() - start))/CLOCKS_PER_SEC);
#endif
  }

  if (Verbose)
    fprintf(stderr,"The edge conflict graph has %d nodes and %d edges\n", C->m, C->nz);

  attach_edge_colors(g, cdim, colors);

 RETURN:
  SparseMatrix_delete(A);
  SparseMatrix_delete(C);
  free(colors);
  free(x);
  if (xsplines){
    for (i = 0; i < ne; i++){
      free(xsplines[i]);
    }
    free(xsplines);
  }
  return g;
}
