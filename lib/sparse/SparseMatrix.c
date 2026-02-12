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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <common/arith.h>
#include <limits.h>
#include <sparse/SparseMatrix.h>
#include <stddef.h>
#include <stdbool.h>
#include <util/alloc.h>
#include <util/overflow.h>
#include <util/prisize_t.h>
#include <util/unreachable.h>

static size_t size_of_matrix_type(int type){
  size_t size = 0;
  switch (type){
  case MATRIX_TYPE_REAL:
    size = sizeof(double);
    break;
  case MATRIX_TYPE_INTEGER:
    size = sizeof(int);
    break;
  case MATRIX_TYPE_PATTERN:
    size = 0;
    break;
  default:
    UNREACHABLE();
  }

  return size;
}

SparseMatrix SparseMatrix_sort(SparseMatrix A){
  SparseMatrix B;
  B = SparseMatrix_transpose(A);
  SparseMatrix_delete(A);
  A = SparseMatrix_transpose(B);
  SparseMatrix_delete(B);
  return A;
}
SparseMatrix SparseMatrix_make_undirected(SparseMatrix A){
  /* make it strictly low diag only, and set flag to undirected */
  SparseMatrix B;
  B = SparseMatrix_symmetrize(A, false);
  B->is_undirected = true;
  return SparseMatrix_remove_upper(B);
}
SparseMatrix SparseMatrix_transpose(SparseMatrix A){
  if (!A) return NULL;

  int *ia = A->ia, *ja = A->ja, *ib, *jb, m = A->m, n = A->n, type = A->type, format = A->format;
  const size_t nz = A->nz;
  SparseMatrix B;
  int j;

  assert(A->format == FORMAT_CSR);/* only implemented for CSR right now */

  B = SparseMatrix_new(n, m, nz, type, format);
  B->nz = nz;
  ib = B->ia;
  jb = B->ja;

  for (int i = 0; i <= n; i++) ib[i] = 0;
  for (int i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      ib[ja[j]+1]++;
    }
  }

  for (int i = 0; i < n; i++) ib[i+1] += ib[i];

  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    double *b = B->a;
    for (int i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]] = i;
	b[ib[ja[j]]++] = a[j];
      }
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *ai = A->a;
    int *bi = B->a;
    for (int i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]] = i;
	bi[ib[ja[j]]++] = ai[j];
      }
    }
    break;
  }
  case MATRIX_TYPE_PATTERN:
    for (int i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]++] = i;
      }
    }
    break;
  default:
    UNREACHABLE();
  }


  for (int i = n-1; i >= 0; i--) ib[i+1] = ib[i];
  ib[0] = 0;
  

  return B;
}

SparseMatrix SparseMatrix_symmetrize(SparseMatrix A,
                                     bool pattern_symmetric_only) {
  SparseMatrix B;
  if (SparseMatrix_is_symmetric(A, pattern_symmetric_only)) return SparseMatrix_copy(A);
  B = SparseMatrix_transpose(A);
  if (!B) return NULL;
  A = SparseMatrix_add(A, B);
  SparseMatrix_delete(B);
  A->is_symmetric = true;
  A->is_pattern_symmetric = true;
  return A;
}

bool SparseMatrix_is_symmetric(SparseMatrix A, bool test_pattern_symmetry_only) {
  if (!A) return false;

  /* assume no repeated entries! */
  SparseMatrix B;
  int *ia, *ja, *ib, *jb, type, m;
  int *mask;
  bool res = false;
  int i, j;
  assert(A->format == FORMAT_CSR);/* only implemented for CSR right now */

  if (A->is_symmetric) return true;
  if (test_pattern_symmetry_only && A->is_pattern_symmetric) return true;

  if (A->m != A->n) return false;

  B = SparseMatrix_transpose(A);
  if (!B) return false;

  ia = A->ia;
  ja = A->ja;
  ib = B->ia;
  jb = B->ja;
  m = A->m;

  mask = gv_calloc((size_t)m, sizeof(int));
  for (i = 0; i < m; i++) mask[i] = -1;

  type = A->type;
  if (test_pattern_symmetry_only) type = MATRIX_TYPE_PATTERN;

  switch (type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    double *b = B->a;
    for (i = 0; i <= m; i++) if (ia[i] != ib[i]) goto RETURN;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = j;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ia[i]) goto RETURN;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (fabs(b[j] - a[mask[jb[j]]]) > SYMMETRY_EPSILON) goto RETURN;
      }
    }
    res = true;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *ai = A->a;
    int *bi = B->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = j;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ia[i]) goto RETURN;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (bi[j] != ai[mask[jb[j]]]) goto RETURN;
      }
    }
    res = true;
    break;
  }
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = j;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ia[i]) goto RETURN;
      }
    }
    res = true;
    break;
  default:
    UNREACHABLE();
  }

  if (!test_pattern_symmetry_only) {
    A->is_symmetric = true;
  }
  A->is_pattern_symmetric = true;
 RETURN:
  free(mask);

  SparseMatrix_delete(B);
  return res;
}

static SparseMatrix SparseMatrix_init(int m, int n, int type, size_t sz, int format){
  SparseMatrix A = gv_alloc(sizeof(struct SparseMatrix_struct));
  A->m = m;
  A->n = n;
  A->nz = 0;
  A->nzmax = 0;
  A->type = type;
  A->size = sz;
  switch (format){
  case FORMAT_COORD:
    A->ia = NULL;
    break;
  case FORMAT_CSR:
  default:
    A->ia = gv_calloc((size_t)(m + 1), sizeof(int));
  }
  A->ja = NULL;
  A->a = NULL;
  A->format = format;
  return A;
}

static void SparseMatrix_alloc(SparseMatrix A, size_t nz) {
  int format = A->format;

  A->a = NULL;
  switch (format){
  case FORMAT_COORD:
    A->ia = gv_calloc(nz, sizeof(int));
    A->ja = gv_calloc(nz, sizeof(int));
    A->a = gv_calloc(nz, A->size);
    break;
  case FORMAT_CSR:
  default:
    A->ja = gv_calloc(nz, sizeof(int));
    if (A->size > 0 && nz > 0) {
      A->a = gv_calloc(nz, A->size);
    }
    break;
  }
  A->nzmax = nz;
}

static SparseMatrix SparseMatrix_realloc(SparseMatrix A, size_t nz) {
  int format = A->format;

  switch (format){
  case FORMAT_COORD:
    A->ia = gv_recalloc(A->ia, A->nzmax, nz, sizeof(int));
    A->ja = gv_recalloc(A->ja, A->nzmax, nz, sizeof(int));
    if (A->size > 0) {
      if (A->a){
	A->a = gv_recalloc(A->a, A->nzmax, nz, A->size);
      } else {
	A->a = gv_calloc(nz, A->size);
      }
    } 
    break;
  case FORMAT_CSR:
  default:
    A->ja = gv_recalloc(A->ja, A->nzmax, nz, sizeof(int));
    if (A->size > 0) {
      if (A->a){
	A->a = gv_recalloc(A->a, A->nzmax, nz, A->size);
      } else {
	A->a = gv_calloc(nz, A->size);
      }
    }
    break;
  }
  A->nzmax = nz;
  return A;
}

SparseMatrix SparseMatrix_new(int m, int n, size_t nz, int type, int format) {
  /* return a sparse matrix skeleton with row dimension m and storage nz. If nz == 0, 
     only row pointers are allocated */
  SparseMatrix A;
  size_t sz;

  sz = size_of_matrix_type(type);
  A = SparseMatrix_init(m, n, type, sz, format);

  if (nz > 0) SparseMatrix_alloc(A, nz);
  return A;

}

/// a generalized version of `SparseMatrix_new`
///
/// Allows elements to be any data structure, not just real/int/complex etc
static SparseMatrix SparseMatrix_general_new(int m, int n, size_t nz, int type,
                                             size_t sz, int format) {
  /* return a sparse matrix skeleton with row dimension m and storage nz. If nz == 0, 
     only row pointers are allocated. this is more general and allow elements to be 
     any data structure, not just real/int/complex etc
  */
  SparseMatrix A;

  A = SparseMatrix_init(m, n, type, sz, format);

  if (nz > 0) SparseMatrix_alloc(A, nz);
  return A;

}

void SparseMatrix_delete(SparseMatrix A){
  if (!A) return;
  free(A->ia);
  free(A->ja);
  free(A->a);
  free(A);
}

static void SparseMatrix_export_csr(FILE *f, SparseMatrix A){
  const int m = A->m;
  
  switch (A->type){
  case MATRIX_TYPE_INTEGER:
    fprintf(f,"%%%%MatrixMarket matrix coordinate integer general\n");
    break;
  default:
    fprintf(stderr, "export of non-integer matrices is unsupported\n");
    abort();
  }

  fprintf(f, "%d %d %" PRISIZE_T "\n", A->m, A->n, A->nz);
  const int *const ia = A->ia;
  const int *const ja = A->ja;
  const int *const ai = A->a;
  for (int i = 0; i < m; i++) {
    for (int j = ia[i]; j < ia[i + 1]; j++) {
      fprintf(f, "%d %d %d\n", i + 1, ja[j] + 1, ai[j]);
   }
  }
}

void SparseMatrix_export(FILE *f, SparseMatrix A){

  switch (A->format){
  case FORMAT_CSR:
    SparseMatrix_export_csr(f, A);
    break;
  case FORMAT_COORD:
    fprintf(stderr, "exporting coordinate format matrices is not supported\n");
    abort();
  default:
    UNREACHABLE();
  }
}


SparseMatrix SparseMatrix_from_coordinate_format(SparseMatrix A){
  /* convert a sparse matrix in coordinate form to one in compressed row form.*/
  int *irn, *jcn;

  void *a = A->a;

  assert(A->format == FORMAT_COORD);
  if (A->format != FORMAT_COORD) {
    return NULL;
  }
  irn = A->ia;
  jcn = A->ja;
  return SparseMatrix_from_coordinate_arrays(A->nz, A->m, A->n, irn, jcn, a, A->type, A->size);

}
SparseMatrix SparseMatrix_from_coordinate_format_not_compacted(SparseMatrix A){
  /* convert a sparse matrix in coordinate form to one in compressed row form.*/
  int *irn, *jcn;

  void *a = A->a;

  assert(A->format == FORMAT_COORD);
  if (A->format != FORMAT_COORD) {
    return NULL;
  }
  irn = A->ia;
  jcn = A->ja;
  return SparseMatrix_from_coordinate_arrays_not_compacted(A->nz, A->m, A->n, irn, jcn, a, A->type, A->size);
}

static SparseMatrix SparseMatrix_from_coordinate_arrays_internal(size_t nz,
                                                                 int m, int n,
                                                                 int *irn,
                                                                 int *jcn,
                                                                 const void *val0,
                                                                 int type,
                                                                 size_t sz,
                                                                 int sum_repeated) {
  /* convert a sparse matrix in coordinate form to one in compressed row form.
     nz: number of entries
     irn: row indices 0-based
     jcn: column indices 0-based
     val values if not NULL
     type: matrix type
  */

  SparseMatrix A = NULL;
  int *ia, *ja;
  double *a;
  int *ai;

  assert(m > 0 && n > 0);

  if (m <=0 || n <= 0) return NULL;
  A = SparseMatrix_general_new(m, n, nz, type, sz, FORMAT_CSR);
  ia = A->ia;
  ja = A->ja;

  for (int i = 0; i <= m; i++){
    ia[i] = 0;
  }

  switch (type){
  case MATRIX_TYPE_REAL: {
    const double *const val = val0;
    a = A->a;
    for (size_t i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	UNREACHABLE();
      }
      ia[irn[i]+1]++;
    }
    for (int i = 0; i < m; i++) ia[i+1] += ia[i];
    for (size_t i = 0; i < nz; i++){
      a[ia[irn[i]]] = val[i];
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (int i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  }
  case MATRIX_TYPE_INTEGER: {
    const int *const vali = val0;
    ai = A->a;
    for (size_t i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	UNREACHABLE();
      }
      ia[irn[i]+1]++;
    }
    for (int i = 0; i < m; i++) ia[i+1] += ia[i];
    for (size_t i = 0; i < nz; i++){
      ai[ia[irn[i]]] = vali[i];
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (int i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  }
  case MATRIX_TYPE_PATTERN:
    for (size_t i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	UNREACHABLE();
      }
      ia[irn[i]+1]++;
    }
    for (int i = 0; i < m; i++) ia[i+1] += ia[i];
    for (size_t i = 0; i < nz; i++){
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (int i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  default:
    UNREACHABLE();
  }
  A->nz = nz;



  if(sum_repeated) A = SparseMatrix_sum_repeat_entries(A);
 
  return A;
}

SparseMatrix SparseMatrix_from_coordinate_arrays(size_t nz, int m, int n,
                                                 int *irn, int *jcn,
                                                 const void *val, int type,
                                                 size_t sz) {
  return SparseMatrix_from_coordinate_arrays_internal(nz, m, n, irn, jcn, val, type, sz, SUM_REPEATED_ALL);
}

SparseMatrix SparseMatrix_from_coordinate_arrays_not_compacted(size_t nz, int m,
                                                               int n, int *irn,
                                                               int *jcn,
                                                               void *val0,
                                                               int type,
                                                               size_t sz) {
  return SparseMatrix_from_coordinate_arrays_internal(nz, m, n, irn, jcn, val0, type, sz, SUM_REPEATED_NONE);
}

SparseMatrix SparseMatrix_add(SparseMatrix A, SparseMatrix B){
  int m, n;
  SparseMatrix C = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic, *jc;
  int j;

  assert(A && B);
  assert(A->format == B->format && A->format == FORMAT_CSR);/* other format not yet supported */
  assert(A->type == B->type);
  m = A->m;
  n = A->n;
  if (m != B->m || n != B->n) return NULL;

  const size_t nzmax = A->nz + B->nz; // just assume that no entries overlaps for speed

  C = SparseMatrix_new(m, n, nzmax, A->type, FORMAT_CSR);
  ic = C->ia;
  jc = C->ja;

  mask = gv_calloc((size_t)n, sizeof(int));

  for (int i = 0; i < n; i++) mask[i] = -1;

  size_t nz = 0;
  ic[0] = 0;
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    double *b = B->a;
    double *c = C->a;
    for (int i = 0; i < m; i++) {
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = (int)nz;
	jc[nz] = ja[j];
	c[nz] = a[j];
	nz++;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ic[i]){
	  jc[nz] = jb[j];
	  c[nz++] = b[j];
	} else {
	  c[mask[jb[j]]] += b[j];
	}
      }
      ic[i + 1] = (int)nz;
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = A->a;
    int *b = B->a;
    int *c = C->a;
    for (int i = 0; i < m; i++) {
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = (int)nz;
	jc[nz] = ja[j];
	c[nz] = a[j];
	nz++;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ic[i]){
	  jc[nz] = jb[j];
	  c[nz] = b[j];
	  nz++;
	} else {
	  c[mask[jb[j]]] += b[j];
	}
      }
      ic[i + 1] = (int)nz;
    }
    break;
  }
  case MATRIX_TYPE_PATTERN:{
    for (int i = 0; i < m; i++) {
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = (int)nz;
	jc[nz] = ja[j];
	nz++;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ic[i]){
	  jc[nz] = jb[j];
	  nz++;
	} 
      }
      ic[i + 1] = (int)nz;
    }
    break;
  }
  default:
    UNREACHABLE();
  }
  C->nz = nz;

  free(mask);

  return C;
}

void SparseMatrix_multiply_dense(SparseMatrix A, const double *v, double *res,
                                 int dim) {
  // A × V, with A dimension m × n, with V a dense matrix of dimension n × dim.
  // v[i×dim×j] gives V[i,j]. Result of dimension m × dim. Real only for now.
  int i, j, k, *ia, *ja, m;
  double *a;

  assert(A->format == FORMAT_CSR);
  assert(A->type == MATRIX_TYPE_REAL);

  a = A->a;
  ia = A->ia;
  ja = A->ja;
  m = A->m;

  for (i = 0; i < m; i++){
    for (k = 0; k < dim; k++) res[i * dim + k] = 0;
    for (j = ia[i]; j < ia[i+1]; j++){
      for (k = 0; k < dim; k++) res[i * dim + k] += a[j] * v[ja[j] *dim + k];
    }
  }
}

void SparseMatrix_multiply_vector(SparseMatrix A, double *v, double **res) {
  /* A v or A^T v. Real only for now. */
  int i, j, *ia, *ja, m;
  double *a, *u = NULL;
  int *ai;
  assert(A->format == FORMAT_CSR);
  assert(A->type == MATRIX_TYPE_REAL || A->type == MATRIX_TYPE_INTEGER);

  ia = A->ia;
  ja = A->ja;
  m = A->m;
  u = *res;

  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = A->a;
    if (v){
      if (!u) u = gv_calloc((size_t)m, sizeof(double));
      for (i = 0; i < m; i++){
	u[i] = 0.;
	for (j = ia[i]; j < ia[i+1]; j++){
	  u[i] += a[j]*v[ja[j]];
	}
      }
    } else {
      /* v is assumed to be all 1's */
      if (!u) u = gv_calloc((size_t)m, sizeof(double));
      for (i = 0; i < m; i++){
	u[i] = 0.;
	for (j = ia[i]; j < ia[i+1]; j++){
	  u[i] += a[j];
	}
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    ai = A->a;
    if (v){
      if (!u) u = gv_calloc((size_t)m, sizeof(double));
      for (i = 0; i < m; i++){
	u[i] = 0.;
	for (j = ia[i]; j < ia[i+1]; j++){
	  u[i] += ai[j]*v[ja[j]];
	}
      }
    } else {
      /* v is assumed to be all 1's */
      if (!u) u = gv_calloc((size_t)m, sizeof(double));
      for (i = 0; i < m; i++){
	u[i] = 0.;
	for (j = ia[i]; j < ia[i+1]; j++){
	  u[i] += ai[j];
	}
      }
    }
    break;
  default:
    UNREACHABLE();
  }
  *res = u;

}

SparseMatrix SparseMatrix_multiply(SparseMatrix A, SparseMatrix B){
  int m;
  SparseMatrix C = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic, *jc;
  int j, k, jj, type;

  assert(A->format == B->format && A->format == FORMAT_CSR);/* other format not yet supported */

  m = A->m;
  if (A->n != B->m) return NULL;
  if (A->type != B->type){
#ifdef DEBUG
    printf("in SparseMatrix_multiply, the matrix types do not match, right now only multiplication of matrices of the same type is supported\n");
#endif
    return NULL;
  }
  type = A->type;
  
  mask = calloc((size_t)B->n, sizeof(int));
  if (!mask) return NULL;

  for (int i = 0; i < B->n; i++) mask[i] = -1;

  size_t nz = 0;
  for (int i = 0; i < m; i++) {
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      for (k = ib[jj]; k < ib[jj+1]; k++){
	if (mask[jb[k]] != -i - 2){
	  if (size_overflow(nz, 1, &nz)) {
#ifdef DEBUG_PRINT
	    fprintf(stderr,"overflow in SparseMatrix_multiply !!!\n");
#endif
	    free(mask);
	    return NULL;
	  }
	  mask[jb[k]] = -i - 2;
	}
      }
    }
  }

  C = SparseMatrix_new(m, B->n, nz, type, FORMAT_CSR);
  ic = C->ia;
  jc = C->ja;
  
  nz = 0;

  switch (type){
  case MATRIX_TYPE_REAL:
    {
      double *a = A->a;
      double *b = B->a;
      double *c = C->a;
      ic[0] = 0;
      for (int i = 0; i < m; i++) {
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (k = ib[jj]; k < ib[jj+1]; k++){
	    if (mask[jb[k]] < ic[i]){
	      mask[jb[k]] = (int)nz;
	      jc[nz] = jb[k];
	      c[nz] = a[j]*b[k];
	      nz++;
	    } else {
	      assert(jc[mask[jb[k]]] == jb[k]);
	      c[mask[jb[k]]] += a[j]*b[k];
	    }
	  }
	}
	ic[i + 1] = (int)nz;
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    {
      int *a = A->a;
      int *b = B->a;
      int *c = C->a;
      ic[0] = 0;
      for (int i = 0; i < m; i++) {
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (k = ib[jj]; k < ib[jj+1]; k++){
	    if (mask[jb[k]] < ic[i]){
	      mask[jb[k]] = (int)nz;
	      jc[nz] = jb[k];
	      c[nz] = a[j]*b[k];
	      nz++;
	    } else {
	      assert(jc[mask[jb[k]]] == jb[k]);
	      c[mask[jb[k]]] += a[j]*b[k];
	    }
	  }
	}
	ic[i + 1] = (int)nz;
      }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    ic[0] = 0;
    for (int i = 0; i < m; i++) {
      for (j = ia[i]; j < ia[i+1]; j++){
	jj = ja[j];
	for (k = ib[jj]; k < ib[jj+1]; k++){
	  if (mask[jb[k]] < ic[i]){
	    mask[jb[k]] = (int)nz;
	    jc[nz] = jb[k];
	    nz++;
	  } else {
	    assert(jc[mask[jb[k]]] == jb[k]);
	  }
	}
      }
      ic[i + 1] = (int)nz;
    }
    break;
  default:
    UNREACHABLE();
  }
  
  C->nz = nz;

  free(mask);
  return C;
}



SparseMatrix SparseMatrix_multiply3(SparseMatrix A, SparseMatrix B, SparseMatrix C){
  int m;
  SparseMatrix D = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic = C->ia, *jc = C->ja, *id, *jd;
  int j, k, l, ll, jj, type;

  assert(A->format == B->format && A->format == FORMAT_CSR);/* other format not yet supported */

  m = A->m;
  if (A->n != B->m) return NULL;
  if (B->n != C->m) return NULL;

  if (A->type != B->type || B->type != C->type){
#ifdef DEBUG
    printf("in SparseMatrix_multiply3, the matrix types do not match, right now only multiplication of matrices of the same type is supported\n");
#endif
    return NULL;
  }
  type = A->type;

  assert(type == MATRIX_TYPE_REAL);

  mask = calloc((size_t)C->n, sizeof(int));
  if (!mask) return NULL;

  for (int i = 0; i < C->n; i++) mask[i] = -1;

  size_t nz = 0;
  for (int i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      for (l = ib[jj]; l < ib[jj+1]; l++){
	ll = jb[l];
	for (k = ic[ll]; k < ic[ll+1]; k++){
	  if (mask[jc[k]] != -i - 2){
	    if (size_overflow(nz, 1, &nz)) {
#ifdef DEBUG_PRINT
	      fprintf(stderr, "overflow in SparseMatrix_multiply3 !!!\n");
#endif
	      free(mask);
	      return NULL;
	    }
	    mask[jc[k]] = -i - 2;
	  }
	}
      }
    }
  }

  D = SparseMatrix_new(m, C->n, nz, type, FORMAT_CSR);
  id = D->ia;
  jd = D->ja;
  
  nz = 0;

  double *a = A->a;
  double *b = B->a;
  double *c = C->a;
  double *d = D->a;
  id[0] = 0;
  for (int i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      for (l = ib[jj]; l < ib[jj+1]; l++){
        ll = jb[l];
        for (k = ic[ll]; k < ic[ll+1]; k++){
          if (mask[jc[k]] < id[i]){
            mask[jc[k]] = (int)nz;
            jd[nz] = jc[k];
            d[nz] = a[j]*b[l]*c[k];
            nz++;
          } else {
            assert(jd[mask[jc[k]]] == jc[k]);
            d[mask[jc[k]]] += a[j]*b[l]*c[k];
          }
        }
      }
    }
    id[i + 1] = (int)nz;
  }
  
  D->nz = nz;

  free(mask);
  return D;
}

SparseMatrix SparseMatrix_sum_repeat_entries(SparseMatrix A){
  /* sum repeated entries in the same row, i.e., {1,1}->1, {1,1}->2 becomes {1,1}->3 */
  int *ia = A->ia, *ja = A->ja, type = A->type, n = A->n;
  int *mask = NULL, j, sta;
  size_t nz = 0;

  mask = gv_calloc((size_t)n, sizeof(int));
  for (int i = 0; i < n; i++) mask[i] = -1;

  switch (type){
  case MATRIX_TYPE_REAL:
    {
      double *a = A->a;
      sta = ia[0];
      for (int i = 0; i < A->m; i++) {
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    a[nz] = a[j];
	    mask[ja[j]] = (int)nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	    a[mask[ja[j]]] += a[j];
	  }
	}
	sta = ia[i+1];
	ia[i + 1] = (int)nz;
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    {
      int *a = A->a;
      sta = ia[0];
      for (int i = 0; i < A->m; i++) {
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    a[nz] = a[j];
	    mask[ja[j]] = (int)nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	    a[mask[ja[j]]] += a[j];
	  }
	}
	sta = ia[i+1];
	ia[i + 1] = (int)nz;
      }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    {
      sta = ia[0];
      for (int i = 0; i < A->m; i++) {
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    mask[ja[j]] = (int)nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	  }
	}
	sta = ia[i+1];
	ia[i + 1] = (int)nz;
      }
    }
    break;
  default:
    free(mask);
    return NULL;
  }
  A->nz = nz;
  free(mask);
  return A;
}

SparseMatrix SparseMatrix_coordinate_form_add_entry_(SparseMatrix A, int irn,
                                                     int jcn, const void *val,
                                                     int type) {
  static const size_t nentries = 1;
  
  assert(A->format == FORMAT_COORD);
  assert(A->type == type && "call to SparseMatrix_coordinate_form_add_entry "
                            "with incompatible value type");
  (void)type;
  const size_t nz = A->nz;

  if (nz + nentries >= A->nzmax){
    const size_t nzmax = nz + nentries + 10;
    A = SparseMatrix_realloc(A, nzmax);
  }
  A->ia[nz] = irn;
  A->ja[nz] = jcn;
  if (A->size) memcpy((char *)A->a + nz * A->size / sizeof(char), val, A->size * nentries);
  if (irn >= A->m) A->m = irn + 1;
  if (jcn >= A->n) A->n = jcn + 1;
  A->nz += nentries;
  return A;
}


SparseMatrix SparseMatrix_remove_diagonal(SparseMatrix A){
  int i, j, *ia, *ja, sta;

  if (!A) return A;

  size_t nz = 0;
  ia = A->ia;
  ja = A->ja;
  sta = ia[0];
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_PATTERN:{
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz++] = ja[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  default:
    UNREACHABLE();
  }

  return A;
}


SparseMatrix SparseMatrix_remove_upper(SparseMatrix A){/* remove diag and upper diag */
  int i, j, *ia, *ja, sta;

  if (!A) return A;

  size_t nz = 0;
  ia = A->ia;
  ja = A->ja;
  sta = ia[0];
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_PATTERN:{
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz++] = ja[j];
	}
      }
      sta = ia[i+1];
      ia[i + 1] = (int)nz;
    }
    A->nz = nz;
    break;
  }
  default:
    UNREACHABLE();
  }

  A->is_pattern_symmetric = false;
  A->is_symmetric = false;
  return A;
}




SparseMatrix SparseMatrix_divide_row_by_degree(SparseMatrix A){
  int i, j, *ia;
  double deg;

  if (!A) return A;

  ia = A->ia;
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    for (i = 0; i < A->m; i++){
      deg = ia[i+1] - ia[i];
      for (j = ia[i]; j < ia[i+1]; j++){
	a[j] = a[j]/deg;
      }
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:
    UNREACHABLE(); // this operation would not make sense for int matrix 
  case MATRIX_TYPE_PATTERN:{
    break;
  }
  default:
    UNREACHABLE();
  }

  return A;
}


SparseMatrix SparseMatrix_get_real_adjacency_matrix_symmetrized(SparseMatrix A){
  /* symmetric, all entries to 1, diaginal removed */
  int *ia, *ja, m, n;
  double *a;
  SparseMatrix B;

  if (!A) return A;
  
  const size_t nz = A->nz;
  ia = A->ia;
  ja = A->ja;
  n = A->n;
  m = A->m;

  if (n != m) return NULL;

  B = SparseMatrix_new(m, n, nz, MATRIX_TYPE_PATTERN, FORMAT_CSR);

  memcpy(B->ia, ia, sizeof(int)*((size_t)(m+1)));
  memcpy(B->ja, ja, sizeof(int) * nz);
  B->nz = A->nz;

  A = SparseMatrix_symmetrize(B, true);
  SparseMatrix_delete(B);
  A = SparseMatrix_remove_diagonal(A);
  A->a = gv_calloc(A->nz, sizeof(double));
  a = A->a;
  for (size_t i = 0; i < A->nz; i++) a[i] = 1.;
  A->type = MATRIX_TYPE_REAL;
  A->size = sizeof(double);
  return A;
}

SparseMatrix SparseMatrix_apply_fun(SparseMatrix A, double (*fun)(double x)){
  int i, j;
  double *a;


  if (!A) return A;
  if (A->format != FORMAT_CSR && A->type != MATRIX_TYPE_REAL) {
#ifdef DEBUG
    printf("only CSR and real matrix supported.\n");
#endif
    return A;
  }


  a = A->a;
  for (i = 0; i < A->m; i++){
    for (j = A->ia[i]; j < A->ia[i+1]; j++){
      a[j] = fun(a[j]);
    }
  }
  return A;
}

SparseMatrix SparseMatrix_copy(SparseMatrix A){
  SparseMatrix B;
  if (!A) return A;
  B = SparseMatrix_general_new(A->m, A->n, A->nz, A->type, A->size, A->format);
  memcpy(B->ia, A->ia, sizeof(int)*((size_t)(A->m+1)));
  if (A->ia[A->m] != 0) {
    memcpy(B->ja, A->ja, sizeof(int)*((size_t)(A->ia[A->m])));
  }
  if (A->a) memcpy(B->a, A->a, A->size * A->nz);
  B->is_pattern_symmetric = A->is_pattern_symmetric;
  B->is_symmetric = A->is_symmetric;
  B->is_undirected = A->is_undirected;
  B->nz = A->nz;
  return B;
}

bool SparseMatrix_has_diagonal(SparseMatrix A) {

  int i, j, m = A->m, *ia = A->ia, *ja = A->ja;

  for (i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      if (i == ja[j]) return true;
    }
  }
  return false;
}

static void SparseMatrix_level_sets(SparseMatrix A, int root, int *nlevel,
                                    int **levelset_ptr, int **levelset,
                                    int **mask, bool reinitialize_mask) {
  /* mask is assumed to be initialized to negative if provided.
     . On exit, mask = levels for visited nodes (1 for root, 2 for its neighbors, etc), 
     . unless reinitialize_mask is true, in which case mask = -1.
     A: the graph, undirected
     root: starting node
     nlevel: max distance to root from any node (in the connected comp)
     levelset_ptr, levelset: the level sets
   */
  int j, sta = 0, sto = 1, ii;
  int m = A->m, *ia = A->ia, *ja = A->ja;

  if (!(*levelset_ptr)) *levelset_ptr = gv_calloc((size_t)(m + 2), sizeof(int));
  if (!(*levelset)) *levelset = gv_calloc((size_t)m, sizeof(int));
  if (!(*mask)) {
    *mask = gv_calloc((size_t)m, sizeof(int));
    for (int i = 0; i < m; i++) (*mask)[i] = UNMASKED;
  }

  *nlevel = 0;
  assert(root >= 0 && root < m);
  (*levelset_ptr)[0] = 0;
  (*levelset_ptr)[1] = 1;
  (*levelset)[0] = root;
  (*mask)[root] = 1;
  *nlevel = 1;
  size_t nz = 1;
  sta = 0; sto = 1;
  while (sto > sta){
    for (int i = sta; i < sto; i++){
      ii = (*levelset)[i];
      for (j = ia[ii]; j < ia[ii+1]; j++){
	if (ii == ja[j]) continue;
	if ((*mask)[ja[j]] < 0){
	  (*levelset)[nz++] = ja[j];
	  (*mask)[ja[j]] = *nlevel + 1;
	}
      }
    }
    (*levelset_ptr)[++(*nlevel)] = (int)nz;
    sta = sto;
    sto = (int)nz;
  }
  (*nlevel)--;
  if (reinitialize_mask) for (int i = 0; i < (*levelset_ptr)[*nlevel]; i++) (*mask)[(*levelset)[i]] = UNMASKED;
}

int *SparseMatrix_weakly_connected_components(SparseMatrix A0, int *ncomp,
                                              int **comps) {
  SparseMatrix A = A0;
  int *levelset_ptr = NULL, *levelset = NULL, *mask = NULL, nlevel;
  int m = A->m, i, nn;

  if (!SparseMatrix_is_symmetric(A, true)){
    A = SparseMatrix_symmetrize(A, true);
  }
  int *comps_ptr = gv_calloc((size_t)(m + 1), sizeof(int));

  *ncomp = 0;
  comps_ptr[0] = 0;
  for (i = 0; i < m; i++){
    if (i == 0 || mask[i] < 0) {
      SparseMatrix_level_sets(A, i, &nlevel, &levelset_ptr, &levelset, &mask, false);
      if (i == 0) *comps = levelset;
      nn = levelset_ptr[nlevel];
      levelset += nn;
      comps_ptr[(*ncomp)+1] = comps_ptr[(*ncomp)] + nn;
      (*ncomp)++;
    }
    
  }
  if (A != A0) SparseMatrix_delete(A);
  free(levelset_ptr);

  free(mask);
  return comps_ptr;
}

void SparseMatrix_decompose_to_supervariables(SparseMatrix A, int *ncluster, int **cluster, int **clusterp){
  /* nodes for a super variable if they share exactly the same neighbors. This is know as modules in graph theory.
     We work on columns only and columns with the same pattern are grouped as a super variable
   */
  int *ia = A->ia, *ja = A->ja, n = A->n, m = A->m;
  int *super = NULL, *nsuper = NULL, j, *mask = NULL, isup, *newmap, isuper;

  super = gv_calloc((size_t)n, sizeof(int));
  nsuper = gv_calloc((size_t)(n + 1), sizeof(int));
  mask = gv_calloc((size_t)n, sizeof(int));
  newmap = gv_calloc((size_t)n, sizeof(int));
  nsuper++;

  isup = 0;
  for (int i = 0; i < n; i++) super[i] = isup;/* every node belongs to super variable 0 by default */
  nsuper[0] = n;
  for (int i = 0; i < n; i++) mask[i] = -1;
  isup++;

  for (int i = 0; i < m; i++){
#ifdef DEBUG_PRINT1
    printf("\n");
    printf("doing row %d-----\n",i+1);
#endif
    for (j = ia[i]; j < ia[i+1]; j++){
      isuper = super[ja[j]];
      nsuper[isuper]--;/* those entries will move to a different super vars*/
    }
    for (j = ia[i]; j < ia[i+1]; j++){
      isuper = super[ja[j]];
      if (mask[isuper] < i){
	mask[isuper] = i;
	if (nsuper[isuper] == 0){/* all nodes in the isuper group exist in this row */
#ifdef DEBUG_PRINT1
	  printf("node %d keep super node id  %d\n",ja[j]+1,isuper+1);
#endif
	  nsuper[isuper] = 1;
	  newmap[isuper] = isuper;
	} else {
	  newmap[isuper] = isup;
	  nsuper[isup] = 1;
#ifdef DEBUG_PRINT1
	  printf("make node %d into supernode %d\n",ja[j]+1,isup+1);
#endif
	  super[ja[j]] = isup++;
	}
      } else {
#ifdef DEBUG_PRINT1
	printf("node %d join super node %d\n",ja[j]+1,newmap[isuper]+1);
#endif
	super[ja[j]] = newmap[isuper];
	nsuper[newmap[isuper]]++;
      }
    }
#ifdef DEBUG_PRINT1
    printf("nsuper=");
    for (j = 0; j < isup; j++) printf("(%d,%d),",j+1,nsuper[j]);
      printf("\n");
#endif
  }
#ifdef DEBUG_PRINT1
  for (int i = 0; i < n; i++){
    printf("node %d is in supernode %d\n",i, super[i]);
  }
#endif
#ifdef PRINT
  fprintf(stderr, "n = %d, nsup = %d\n",n,isup);
#endif
  /* now accumulate super nodes */
  nsuper--;
  nsuper[0] = 0;
  for (int i = 0; i < isup; i++) nsuper[i+1] += nsuper[i];

  *cluster = newmap;
  for (int i = 0; i < n; i++) {
    isuper = super[i];
    (*cluster)[nsuper[isuper]++] = i;
  }
  for (int i = isup; i > 0; i--) nsuper[i] = nsuper[i-1];
  nsuper[0] = 0;
  *clusterp = nsuper;
  *ncluster = isup;

#ifdef PRINT
  for (int i = 0; i < *ncluster; i++) {
    printf("{");
    for (j = (*clusterp)[i]; j < (*clusterp)[i+1]; j++){
      printf("%d, ",(*cluster)[j]);
    }
    printf("},");
  }
  printf("\n");
#endif

  free(mask);
  free(super);
}

SparseMatrix SparseMatrix_get_augmented(SparseMatrix A){
  /* convert matrix A to an augmente dmatrix {{0,A},{A^T,0}} */
  int *irn = NULL, *jcn = NULL;
  void *val = NULL;
  size_t nz = A->nz;
  int type = A->type;
  int m = A->m, n = A->n, i, j;
  SparseMatrix B = NULL;
  if (!A) return NULL;
  if (nz > 0){
    irn = gv_calloc(nz * 2, sizeof(int));
    jcn = gv_calloc(nz * 2, sizeof(int));
  }

  if (A->a){
    assert(A->size != 0 && nz > 0);
    val = gv_calloc(2 * nz, A->size);
    memcpy(val, A->a, A->size * nz);
    memcpy((char *)val + nz * A->size, A->a, A->size * nz);
  }

  nz = 0;
  for (i = 0; i < m; i++){
    for (j = (A->ia)[i]; j <  (A->ia)[i+1]; j++){
      irn[nz] = i;
      jcn[nz++] = (A->ja)[j] + m;
    }
  }
  for (i = 0; i < m; i++){
    for (j = (A->ia)[i]; j <  (A->ia)[i+1]; j++){
      jcn[nz] = i;
      irn[nz++] = (A->ja)[j] + m;
    }
  }

  B = SparseMatrix_from_coordinate_arrays(nz, m + n, m + n, irn, jcn, val, type, A->size);
  B->is_symmetric = true;
  B->is_pattern_symmetric = true;
  free(irn);
  free(jcn);
  free(val);
  return B;
}

SparseMatrix SparseMatrix_to_square_matrix(SparseMatrix A, int bipartite_options){
  SparseMatrix B;
  switch (bipartite_options){
  case BIPARTITE_RECT:
    if (A->m == A->n) return A;
    break;
  case BIPARTITE_PATTERN_UNSYM:
    if (A->m == A->n && SparseMatrix_is_symmetric(A, true)) return A;
    break;
  case BIPARTITE_UNSYM:
    if (A->m == A->n && SparseMatrix_is_symmetric(A, false)) return A;
    break;
  case BIPARTITE_ALWAYS:
    break;
  default:
    UNREACHABLE();
  }
  B = SparseMatrix_get_augmented(A);
  SparseMatrix_delete(A);
  return B;
}

SparseMatrix SparseMatrix_get_submatrix(SparseMatrix A, int nrow, int ncol, int *rindices, int *cindices){
  /* get the submatrix from row/columns indices[0,...,l-1]. 
     row rindices[i] will be the new row i
     column cindices[i] will be the new column i.
     if rindices = NULL, it is assume that 1 -- nrow is needed. Same for cindices/ncol.
   */
  size_t nz = 0;
  int j, *irn, *jcn, *ia = A->ia, *ja = A->ja, m = A->m, n = A->n;
  int *cmask, *rmask;
  void *v = NULL;
  SparseMatrix B = NULL;
  int irow = 0, icol = 0;

  if (nrow <= 0 || ncol <= 0) return NULL;

  

  rmask = gv_calloc((size_t)m, sizeof(int));
  cmask = gv_calloc((size_t)n, sizeof(int));
  for (int i = 0; i < m; i++) rmask[i] = -1;
  for (int i = 0; i < n; i++) cmask[i] = -1;

  if (rindices){
    for (int i = 0; i < nrow; i++) {
      if (rindices[i] >= 0 && rindices[i] < m){
	rmask[rindices[i]] = irow++;
      }
    }
  } else {
    for (int i = 0; i < nrow; i++) {
      rmask[i] = irow++;
    }
  }

  if (cindices){
    for (int i = 0; i < ncol; i++) {
      if (cindices[i] >= 0 && cindices[i] < n){
	cmask[cindices[i]] = icol++;
      }
    }
  } else {
    for (int i = 0; i < ncol; i++) {
      cmask[i] = icol++;
    }
  }

  for (int i = 0; i < m; i++) {
    if (rmask[i] < 0) continue;
    for (j = ia[i]; j < ia[i+1]; j++){
      if (cmask[ja[j]] < 0) continue;
      nz++;
    }
  }


  switch (A->type){
  case MATRIX_TYPE_REAL:{
    double *a = A->a;
    double *val;
    irn = gv_calloc(nz, sizeof(int));
    jcn = gv_calloc(nz, sizeof(int));
    val = gv_calloc(nz, sizeof(double));

    nz = 0;
    for (int i = 0; i < m; i++) {
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz] = cmask[ja[j]];
	val[nz++] = a[j];
      }
    }
    v = val;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = A->a;
    int *val;

    irn = gv_calloc(nz, sizeof(int));
    jcn = gv_calloc(nz, sizeof(int));
    val = gv_calloc(nz, sizeof(int));

    nz = 0;
    for (int i = 0; i < m; i++) {
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz] = cmask[ja[j]];
	val[nz] = a[j];
	nz++;
      }
    }
    v = val;
    break;
  }
  case MATRIX_TYPE_PATTERN:
    irn = gv_calloc(nz, sizeof(int));
    jcn = gv_calloc(nz, sizeof(int));
    nz = 0;
     for (int i = 0; i < m; i++) {
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz++] = cmask[ja[j]];
      }
    }
    break;
  default:
    UNREACHABLE();
  }

  B = SparseMatrix_from_coordinate_arrays(nz, nrow, ncol, irn, jcn, v, A->type, A->size);
  free(cmask);
  free(rmask);
  free(irn);
  free(jcn);
  if (v) free(v);


  return B;

}

SparseMatrix SparseMatrix_set_entries_to_real_one(SparseMatrix A){
  double *a;

  free(A->a);
  A->a = gv_calloc(A->nz, sizeof(double));
  a = A->a;
  for (size_t i = 0; i < A->nz; i++) a[i] = 1.;
  A->type = MATRIX_TYPE_REAL;
  A->size = sizeof(double);
  return A;

}

SparseMatrix SparseMatrix_distance_matrix(SparseMatrix D0) {
  SparseMatrix D = D0;
  int m = D->m, n = D->n;
  int *levelset_ptr = NULL, *levelset = NULL, *mask = NULL;
  int i, j, k, nlevel;

  if (!SparseMatrix_is_symmetric(D, false)){
    D = SparseMatrix_symmetrize(D, false);
  }

  assert(m == n);
  (void)m;

  SparseMatrix dist = SparseMatrix_new(n, n, (size_t)n * (size_t)n,
                                       MATRIX_TYPE_INTEGER, FORMAT_CSR);
  int *const d = dist->a;
  for (i = 0; i <= n; ++i) {
    dist->ia[i] = i * n;
  }
  for (i = 0; i < n; ++i) {
    for (j = 0; j < n; ++j) {
      dist->ja[i * n + j] = j;
      d[i * n + j] = -1;
    }
  }

  for (k = 0; k < n; k++) {
    SparseMatrix_level_sets(D, k, &nlevel, &levelset_ptr, &levelset, &mask, true);
    assert(levelset_ptr[nlevel] == n);
    for (i = 0; i < nlevel; i++) {
      for (j = levelset_ptr[i]; j < levelset_ptr[i+1]; j++) {
        d[k * n + levelset[j]] = i;
      }
    }
  }

  free(levelset_ptr);
  free(levelset);
  free(mask);
  
  if (D != D0) SparseMatrix_delete(D);
  return dist;
}
