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

#define STANDALONE
#include "country_graph_coloring.h"
#include <math.h>
#include "power.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <util/debug.h>

static double get_local_12_norm(int n, int i, const int *ia, const int *ja,
                                const int *p) {
  double norm = n;
  for (int j = ia[i]; j < ia[i+1]; j++){
    if (ja[j] == i) continue;
    norm = fmin(norm, abs(p[i] - p[ja[j]]));
  }
  return norm;
}

static void get_12_norm(int n, const int *ia, const int *ja, int *p,
                        double *norm) {
  /* norm[0] := antibandwidth
     norm[1] := (\sum_{i\in V} (Min_{{j,i}\in E} |p[i] - p[j]|)/|V|
  */
  norm[0] = n; norm[1] = 0;
  for (int i = 0; i < n; i++){
    double tmp = n;
    for (int j = ia[i]; j < ia[i+1]; j++){
      if (ja[j] == i) continue;
      norm[0] = fmin(norm[0], abs(p[i] - p[ja[j]]));
      tmp = fmin(tmp, abs(p[i] - p[ja[j]]));
    }
    norm[1] += tmp;
  }
  norm[1] /= n;
}

void improve_antibandwidth_by_swapping(SparseMatrix A, int *p){
  int cnt = 1, n = A->m, *ia = A->ia, *ja = A->ja;
  double norm1[2];
  clock_t start = clock();
  FILE *fp = NULL;
  
  if (Verbose){
    fprintf(stderr,"saving timing vs antiband data to timing_greedy\n");
    fp = fopen("timing_greedy","w");
  }
  assert(SparseMatrix_is_symmetric(A, true));
  for (bool improved = true; improved; ) {
    improved = false;
    for (int i = 0; i < n; i++) {
      norm1[0] = get_local_12_norm(n, i, ia, ja, p);
      for (int j = 0; j < n; j++) {
	if (j == i) continue;
	const double norm2 = get_local_12_norm(n, j, ia, ja, p);
	const int pi = p[i];
	const int pj = p[j];
	p[i] = pj;
	p[j] = pi;
	const double norm11 = get_local_12_norm(n, i, ia, ja, p);
	const double norm22 = get_local_12_norm(n, j, ia, ja, p);
	if (fmin(norm11, norm22) > fmin(norm1[0], norm2)){
	  improved = true;
	  norm1[0] = norm11;
	  continue;
	}
	p[i] = pi;
	p[j] = pj;
      }
      if (i%100 == 0 && Verbose) {
	get_12_norm(n, ia, ja, p, norm1);
	fprintf(fp, "%f %f %f\n", ((double)(clock() - start)) / CLOCKS_PER_SEC,
	        norm1[0], norm1[1]);
      }
    }
    if (Verbose) {
      get_12_norm(n, ia, ja, p, norm1);
      fprintf(stderr, "[%d] aband = %f, aband_avg = %f\n", cnt++, norm1[0], norm1[1]);
      fprintf(fp,"%f %f %f\n", ((double)(clock() - start)) / CLOCKS_PER_SEC,
              norm1[0], norm1[1]);
    }
  }
  if (fp != NULL) {
    fclose(fp);
  }
}
  
void country_graph_coloring(int seed, SparseMatrix A, int **p) {
  int n = A->m;

  clock_t start = clock();
  assert(A->m == A->n);
  SparseMatrix A2 = SparseMatrix_symmetrize(A, true);
  const int *const ia = A2->ia;
  const int *const ja = A2->ja;

  /* Laplacian */
  SparseMatrix L = SparseMatrix_new(n, n, 1, MATRIX_TYPE_REAL, FORMAT_COORD);
  for (int i = 0; i < n; i++){
    double nrow = 0.;
    for (int j = ia[i]; j < ia[i+1]; j++){
      const int jj = ja[j];
      if (jj != i){
	nrow ++;
	L = SparseMatrix_coordinate_form_add_entry(L, i, jj, &(double){-1});
      }
    }
    L = SparseMatrix_coordinate_form_add_entry(L, i, i, &nrow);
  }
  {
    SparseMatrix new = SparseMatrix_from_coordinate_format(L);
    SparseMatrix_delete(L);
    L = new;
  }

  /* largest eigen vector */
  double *v = power_method(L, L->n, seed);

  vector_ordering(n, v, p);
  free(v);
  if (Verbose)
    fprintf(stderr, "cpu time for spectral ordering (before greedy) = %f\n",
            ((double)(clock() - start)) / CLOCKS_PER_SEC);

  clock_t start2 = clock();
  /* swapping */
  improve_antibandwidth_by_swapping(A2, *p);
  if (Verbose) {
    fprintf(stderr, "cpu time for greedy refinement = %f\n",
            ((double)(clock() - start2)) / CLOCKS_PER_SEC);

    fprintf(stderr, "cpu time for spectral + greedy = %f\n",
            ((double)(clock() - start)) / CLOCKS_PER_SEC);

  }

  if (A2 != A) SparseMatrix_delete(A2);
  SparseMatrix_delete(L);
}
