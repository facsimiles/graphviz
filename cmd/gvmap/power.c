/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "power.h"
#include <sparse/SparseMatrix.h>

// Maxium of iterations that will be done in power_method
static const int maxit = 100;

// Accuracy control (convergence criterion) for power_method
static const real tolerance = 0.00001;

void power_method(void *A, int n, int K, int random_seed,
    real **eigv, real *eigs){
  /* find k-largest eigenvectors of a matrix A. Result in eigv. if eigv == NULL; memory will be allocated.

     This converges only if the largest eigenvectors/values are real (e.g., if A is symmetric) and the 
     next largest eigenvalues separate from the largest ones

     input:
     A: the matrix
     n: dimension of matrix A
     K: number of eigenes to find
     random_seed: seed for eigenvector initialization
     matrix: max number f iterations

     output:
     eigv: eigenvectors. The i-th is at eigvs[i*n, i*(n+1) - 1]
     eigs: eigenvalues.  The i-th is at eigs[i]


     Function PowerIteration (A – m × m matrix )
     % This function computes u1, u2, . . . , uk, the first k eigenvectors of S.
     const tolerance ← 0.001
     for i = 1 to k do
     . ui ← random
     . ui ← ui/||ui||
     . do
     .   vi ← ui
     .   % orthogonalize against previous eigenvectors
     .   for j = 1 to i − 1 do
     .     vi ← vi − (vi^Tvi)vj
     .   end for
     .   ui ← A vi/||A vi||
     . while (ui^T vi < 1-tolerance) (halt when direction change is small)
     . vi = ui
     end for
     return v1,v2,...
   */
  real **v, *u, *vv;
  int iter = 0;
  real res, unorm;
  int i, j, k;
  real uij;

  K = MAX(0, MIN(n, K));
  assert(K <= n && K > 0);

  if (!(*eigv)) *eigv = MALLOC(sizeof(real)*n*K);
  v = MALLOC(sizeof(real*)*K);

  vv = MALLOC(sizeof(real)*n);
  u = MALLOC(sizeof(real)*n);

  srand(random_seed);

  for (k = 0; k < K; k++){
    v[k] = &((*eigv)[k*n]);
    for (i = 0; i < n; i++) u[i] = drand();
    res = sqrt(vector_product(n, u, u));
    if (res > 0) res =  1/res;
    for (i = 0; i < n; i++) {
      u[i] = u[i]*res;
      v[k][i] = u[i];
    }
    iter = 0;
    do {


      /* normalize against previous eigens */
      for (j = 0; j < k; j++){
	uij = vector_product(n, u, v[j]);
	for (i = 0; i < n; i++) {
	  u[i] = u[i] - uij *v[j][i];
	}
      }
      SparseMatrix_multiply_vector(A, u, &vv, FALSE);

      unorm = vector_product(n, vv, vv);/* ||u||^2 */    
      unorm = sqrt(unorm);
      eigs[k] = unorm;
      if (unorm > 0) {
	unorm = 1/unorm;
      } else {
	// ||A.v||=0, so v must be an eigenvec correspond to eigenvalue zero
	for (i = 0; i < n; i++) vv[i] = u[i];
	unorm = sqrt(vector_product(n, vv, vv));
	if (unorm > 0) unorm = 1/unorm;
      }
      res = 0.;

      for (i = 0; i < n; i++) {
	u[i] = vv[i]*unorm;
	res = res + u[i]*v[k][i];
	v[k][i] = u[i];
      }
    } while (res < 1 - tolerance && iter++ < maxit);
  }
  free(u);
  free(vv);
}
