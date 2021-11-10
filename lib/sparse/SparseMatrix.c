/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <common/logic.h>
#include <common/memory.h>
#include <common/arith.h>
#include <limits.h>
#include <sparse/SparseMatrix.h>
#include <sparse/BinaryHeap.h>
#include <stddef.h>

static size_t size_of_matrix_type(int type){
  size_t size = 0;
  switch (type){
  case MATRIX_TYPE_REAL:
    size = sizeof(real);
    break;
  case MATRIX_TYPE_COMPLEX:
    size = 2*sizeof(real);
    break;
  case MATRIX_TYPE_INTEGER:
    size = sizeof(int);
    break;
  case MATRIX_TYPE_PATTERN:
    size = 0;
    break;
  case MATRIX_TYPE_UNKNOWN:
    size = 0;
    break;
  default:
    size = 0;
    break;
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
  B = SparseMatrix_symmetrize(A, FALSE);
  SparseMatrix_set_undirected(B);
  return SparseMatrix_remove_upper(B);
}
SparseMatrix SparseMatrix_transpose(SparseMatrix A){
  if (!A) return NULL;

  int *ia = A->ia, *ja = A->ja, *ib, *jb, nz = A->nz, m = A->m, n = A->n, type = A->type, format = A->format;
  SparseMatrix B;
  int i, j;

  assert(A->format == FORMAT_CSR);/* only implemented for CSR right now */

  B = SparseMatrix_new(n, m, nz, type, format);
  B->nz = nz;
  ib = B->ia;
  jb = B->ja;

  for (i = 0; i <= n; i++) ib[i] = 0;
  for (i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      ib[ja[j]+1]++;
    }
  }

  for (i = 0; i < n; i++) ib[i+1] += ib[i];

  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]] = i;
	b[ib[ja[j]]++] = a[j];
      }
    }
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]] = i;
	b[2*ib[ja[j]]] = a[2*j];
	b[2*ib[ja[j]]+1] = a[2*j+1];
	ib[ja[j]]++;
      }
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *ai = (int*) A->a;
    int *bi = (int*) B->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]] = i;
	bi[ib[ja[j]]++] = ai[j];
      }
    }
    break;
  }
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jb[ib[ja[j]]++] = i;
      }
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
    SparseMatrix_delete(B);
    return NULL;
  default:
    SparseMatrix_delete(B);
    return NULL;
  }


  for (i = n-1; i >= 0; i--) ib[i+1] = ib[i];
  ib[0] = 0;
  

  return B;
}

SparseMatrix SparseMatrix_symmetrize(SparseMatrix A, int pattern_symmetric_only){
  SparseMatrix B;
  if (SparseMatrix_is_symmetric(A, pattern_symmetric_only)) return SparseMatrix_copy(A);
  B = SparseMatrix_transpose(A);
  if (!B) return NULL;
  A = SparseMatrix_add(A, B);
  SparseMatrix_delete(B);
  SparseMatrix_set_symmetric(A);
  SparseMatrix_set_pattern_symmetric(A);
  return A;
}

SparseMatrix SparseMatrix_symmetrize_nodiag(SparseMatrix A){
  SparseMatrix B;
  if (SparseMatrix_is_symmetric(A, FALSE)) {
    B = SparseMatrix_copy(A);
    return SparseMatrix_remove_diagonal(B);
  }
  B = SparseMatrix_transpose(A);
  if (!B) return NULL;
  A = SparseMatrix_add(A, B);
  SparseMatrix_delete(B);
  SparseMatrix_set_symmetric(A);
  SparseMatrix_set_pattern_symmetric(A);
  return SparseMatrix_remove_diagonal(A);
}

int SparseMatrix_is_symmetric(SparseMatrix A, int test_pattern_symmetry_only){
  if (!A) return FALSE;

  /* assume no repeated entries! */
  SparseMatrix B;
  int *ia, *ja, *ib, *jb, type, m;
  int *mask;
  int res = FALSE;
  int i, j;
  assert(A->format == FORMAT_CSR);/* only implemented for CSR right now */

  if (SparseMatrix_known_symmetric(A)) return TRUE;
  if (test_pattern_symmetry_only && SparseMatrix_known_strucural_symmetric(A)) return TRUE;

  if (A->m != A->n) return FALSE;

  B = SparseMatrix_transpose(A);
  if (!B) return FALSE;

  ia = A->ia;
  ja = A->ja;
  ib = B->ia;
  jb = B->ja;
  m = A->m;

  mask = MALLOC(sizeof(int)*((size_t) m));
  for (i = 0; i < m; i++) mask[i] = -1;

  type = A->type;
  if (test_pattern_symmetry_only) type = MATRIX_TYPE_PATTERN;

  switch (type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
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
    res = TRUE;
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
    for (i = 0; i <= m; i++) if (ia[i] != ib[i]) goto RETURN;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = j;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ia[i]) goto RETURN;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (fabs(b[2*j] - a[2*mask[jb[j]]]) > SYMMETRY_EPSILON) goto RETURN;
	if (fabs(b[2*j+1] - a[2*mask[jb[j]]+1]) > SYMMETRY_EPSILON) goto RETURN;
      }
    }
    res = TRUE;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *ai = (int*) A->a;
    int *bi = (int*) B->a;
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
    res = TRUE;
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
    res = TRUE;
    break;
  case MATRIX_TYPE_UNKNOWN:
    goto RETURN;
    break;
  default:
    goto RETURN;
    break;
  }

  if (test_pattern_symmetry_only){
    SparseMatrix_set_pattern_symmetric(A);
  } else {
    SparseMatrix_set_symmetric(A);
    SparseMatrix_set_pattern_symmetric(A);
  }
 RETURN:
  free(mask);

  SparseMatrix_delete(B);
  return res;
}

static SparseMatrix SparseMatrix_init(int m, int n, int type, size_t sz, int format){
  SparseMatrix A;


  A = MALLOC(sizeof(struct SparseMatrix_struct));
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
  case FORMAT_CSC:
  case FORMAT_CSR:
  default:
    A->ia = MALLOC(sizeof(int)*((size_t)(m+1)));
  }
  A->ja = NULL;
  A->a = NULL;
  A->format = format;
  A->property = 0;
  clear_flag(A->property, MATRIX_PATTERN_SYMMETRIC);
  clear_flag(A->property, MATRIX_SYMMETRIC);
  clear_flag(A->property, MATRIX_SKEW);
  clear_flag(A->property, MATRIX_HERMITIAN);
  return A;
}

static SparseMatrix SparseMatrix_alloc(SparseMatrix A, int nz){
  int format = A->format;
  size_t nz_t = (size_t) nz; /* size_t is 64 bit on 64 bit machine. Using nz*A->size can overflow. */

  A->a = NULL;
  switch (format){
  case FORMAT_COORD:
    A->ia = MALLOC(sizeof(int)*nz_t);
    A->ja = MALLOC(sizeof(int)*nz_t);
    A->a = MALLOC(A->size*nz_t);
    break;
  case FORMAT_CSR:
  case FORMAT_CSC:
  default:
    A->ja = MALLOC(sizeof(int)*nz_t);
    if (A->size > 0 && nz_t > 0) {
      A->a = MALLOC(A->size*nz_t);
    }
    break;
  }
  A->nzmax = nz;
  return A;
}

static SparseMatrix SparseMatrix_realloc(SparseMatrix A, int nz){
  int format = A->format;
  size_t nz_t = (size_t) nz; /* size_t is 64 bit on 64 bit machine. Using nz*A->size can overflow. */

  switch (format){
  case FORMAT_COORD:
    A->ia = REALLOC(A->ia, sizeof(int)*nz_t);
    A->ja = REALLOC(A->ja, sizeof(int)*nz_t);
    if (A->size > 0) {
      if (A->a){
	A->a = REALLOC(A->a, A->size*nz_t);
      } else {
	A->a = MALLOC(A->size*nz_t);
      }
    } 
    break;
  case FORMAT_CSR:
  case FORMAT_CSC:
  default:
    A->ja = REALLOC(A->ja, sizeof(int)*nz_t);
    if (A->size > 0) {
      if (A->a){
	A->a = REALLOC(A->a, A->size*nz_t);
      } else {
	A->a = MALLOC(A->size*nz_t);
      }
    }
    break;
  }
  A->nzmax = nz;
  return A;
}

SparseMatrix SparseMatrix_new(int m, int n, int nz, int type, int format){
  /* return a sparse matrix skeleton with row dimension m and storage nz. If nz == 0, 
     only row pointers are allocated */
  SparseMatrix A;
  size_t sz;

  sz = size_of_matrix_type(type);
  A = SparseMatrix_init(m, n, type, sz, format);

  if (nz > 0) A = SparseMatrix_alloc(A, nz);
  return A;

}
SparseMatrix SparseMatrix_general_new(int m, int n, int nz, int type, size_t sz, int format){
  /* return a sparse matrix skeleton with row dimension m and storage nz. If nz == 0, 
     only row pointers are allocated. this is more general and allow elements to be 
     any data structure, not just real/int/complex etc
  */
  SparseMatrix A;

  A = SparseMatrix_init(m, n, type, sz, format);

  if (nz > 0) A = SparseMatrix_alloc(A, nz);
  return A;

}

void SparseMatrix_delete(SparseMatrix A){
  /* return a sparse matrix skeleton with row dimension m and storage nz. If nz == 0, 
     only row pointers are allocated */
  if (!A) return;
  free(A->ia);
  free(A->ja);
  free(A->a);
  free(A);
}
static void SparseMatrix_print_csr(char *c, SparseMatrix A){
  int *ia, *ja;
  real *a;
  int *ai;
  int i, j, m = A->m;
  
  assert (A->format == FORMAT_CSR);
  printf("%s\n SparseArray[{",c);
  ia = A->ia;
  ja = A->ja;
  a = A->a;
  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	printf("{%d, %d}->%f",i+1, ja[j]+1, a[j]);
	if (j != ia[m]-1) printf(",");
      }
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    a = (real*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	printf("{%d, %d}->%f + %f I",i+1, ja[j]+1, a[2*j], a[2*j+1]);
 	if (j != ia[m]-1) printf(",");
     }
    }
    printf("\n");
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	printf("{%d, %d}->%d",i+1, ja[j]+1, ai[j]);
 	if (j != ia[m]-1) printf(",");
     }
    }
    printf("\n");
    break;
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	printf("{%d, %d}->_",i+1, ja[j]+1);
 	if (j != ia[m]-1) printf(",");
      }
    }
    printf("\n");
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }
  printf("},{%d, %d}]\n", m, A->n);

}



static void SparseMatrix_print_coord(char *c, SparseMatrix A){
  int *ia, *ja;
  real *a;
  int *ai;
  int i, m = A->m;
  
  assert (A->format == FORMAT_COORD);
  printf("%s\n SparseArray[{",c);
  ia = A->ia;
  ja = A->ja;
  a = A->a;
  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    for (i = 0; i < A->nz; i++){
      printf("{%d, %d}->%f",ia[i]+1, ja[i]+1, a[i]);
      if (i != A->nz - 1) printf(",");
    }
    printf("\n");
    break;
  case MATRIX_TYPE_COMPLEX:
    a = (real*) A->a;
    for (i = 0; i < A->nz; i++){
      printf("{%d, %d}->%f + %f I",ia[i]+1, ja[i]+1, a[2*i], a[2*i+1]);
      if (i != A->nz - 1) printf(",");
    }
    printf("\n");
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    for (i = 0; i < A->nz; i++){
      printf("{%d, %d}->%d",ia[i]+1, ja[i]+1, ai[i]);
      if (i != A->nz) printf(",");
    }
    printf("\n");
    break;
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < A->nz; i++){
      printf("{%d, %d}->_",ia[i]+1, ja[i]+1);
      if (i != A->nz - 1) printf(",");
    }
    printf("\n");
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }
  printf("},{%d, %d}]\n", m, A->n);

}




void SparseMatrix_print(char *c, SparseMatrix A){
  switch (A->format){
  case FORMAT_CSR:
    SparseMatrix_print_csr(c, A);
    break;
  case FORMAT_CSC:
    assert(0);/* not implemented yet...
		 SparseMatrix_print_csc(c, A);*/
    break;
  case FORMAT_COORD:
    SparseMatrix_print_coord(c, A);
    break;
  default:
    assert(0);
  }
}





static void SparseMatrix_export_csr(FILE *f, SparseMatrix A){
  int *ia, *ja;
  real *a;
  int *ai;
  int i, j, m = A->m;
  
  switch (A->type){
  case MATRIX_TYPE_REAL:
    fprintf(f,"%%%%MatrixMarket matrix coordinate real general\n");
    break;
  case MATRIX_TYPE_COMPLEX:
    fprintf(f,"%%%%MatrixMarket matrix coordinate complex general\n");
    break;
  case MATRIX_TYPE_INTEGER:
    fprintf(f,"%%%%MatrixMarket matrix coordinate integer general\n");
    break;
  case MATRIX_TYPE_PATTERN:
    fprintf(f,"%%%%MatrixMarket matrix coordinate pattern general\n");
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }

  fprintf(f,"%d %d %d\n",A->m,A->n,A->nz);
  ia = A->ia;
  ja = A->ja;
  a = A->a;
  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	fprintf(f, "%d %d %16.8g\n",i+1, ja[j]+1, a[j]);
      }
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    a = (real*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	fprintf(f, "%d %d %16.8g %16.8g\n",i+1, ja[j]+1, a[2*j], a[2*j+1]);
     }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	fprintf(f, "%d %d %d\n",i+1, ja[j]+1, ai[j]);
     }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	fprintf(f, "%d %d\n",i+1, ja[j]+1);
      }
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }

}

static void SparseMatrix_export_coord(FILE *f, SparseMatrix A){
  int *ia, *ja;
  real *a;
  int *ai;
  int i;
  
  switch (A->type){
  case MATRIX_TYPE_REAL:
    fprintf(f,"%%%%MatrixMarket matrix coordinate real general\n");
    break;
  case MATRIX_TYPE_COMPLEX:
    fprintf(f,"%%%%MatrixMarket matrix coordinate complex general\n");
    break;
  case MATRIX_TYPE_INTEGER:
    fprintf(f,"%%%%MatrixMarket matrix coordinate integer general\n");
    break;
  case MATRIX_TYPE_PATTERN:
    fprintf(f,"%%%%MatrixMarket matrix coordinate pattern general\n");
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }

  fprintf(f,"%d %d %d\n",A->m,A->n,A->nz);
  ia = A->ia;
  ja = A->ja;
  a = A->a;
  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    for (i = 0; i < A->nz; i++){
      fprintf(f, "%d %d %16.8g\n",ia[i]+1, ja[i]+1, a[i]);
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    a = (real*) A->a;
    for (i = 0; i < A->nz; i++){
      fprintf(f, "%d %d %16.8g %16.8g\n",ia[i]+1, ja[i]+1, a[2*i], a[2*i+1]);
    }
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    for (i = 0; i < A->nz; i++){
      fprintf(f, "%d %d %d\n",ia[i]+1, ja[i]+1, ai[i]);
    }
    break;
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < A->nz; i++){
      fprintf(f, "%d %d\n",ia[i]+1, ja[i]+1);
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
    return;
  default:
    return;
  }
}



void SparseMatrix_export(FILE *f, SparseMatrix A){

  switch (A->format){
  case FORMAT_CSR:
    SparseMatrix_export_csr(f, A);
    break;
  case FORMAT_CSC:
    assert(0);/* not implemented yet...
		 SparseMatrix_print_csc(c, A);*/
    break;
  case FORMAT_COORD:
    SparseMatrix_export_coord(f, A);
    break;
  default:
    assert(0);
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

static SparseMatrix SparseMatrix_from_coordinate_arrays_internal(int nz, int m, int n, int *irn, int *jcn, void *val0, int type, size_t sz, int sum_repeated){
  /* convert a sparse matrix in coordinate form to one in compressed row form.
     nz: number of entries
     irn: row indices 0-based
     jcn: column indices 0-based
     val values if not NULL
     type: matrix type
  */

  SparseMatrix A = NULL;
  int *ia, *ja;
  real *a, *val;
  int *ai, *vali;
  int i;

  assert(m > 0 && n > 0 && nz >= 0);

  if (m <=0 || n <= 0 || nz < 0) return NULL;
  A = SparseMatrix_general_new(m, n, nz, type, sz, FORMAT_CSR);
  assert(A);
  if (!A) return NULL;
  ia = A->ia;
  ja = A->ja;

  for (i = 0; i <= m; i++){
    ia[i] = 0;
  }

  switch (type){
  case MATRIX_TYPE_REAL:
    val = (real*) val0;
    a = (real*) A->a;
    for (i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	assert(0);
	return NULL;
      }
      ia[irn[i]+1]++;
    }
    for (i = 0; i < m; i++) ia[i+1] += ia[i];
    for (i = 0; i < nz; i++){
      a[ia[irn[i]]] = val[i];
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  case MATRIX_TYPE_COMPLEX:
    val = (real*) val0;
    a = (real*) A->a;
    for (i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	assert(0);
	return NULL;
      }
      ia[irn[i]+1]++;
    }
    for (i = 0; i < m; i++) ia[i+1] += ia[i];
    for (i = 0; i < nz; i++){
      a[2*ia[irn[i]]] = *(val++);
      a[2*ia[irn[i]]+1] = *(val++);
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  case MATRIX_TYPE_INTEGER:
    vali = (int*) val0;
    ai = (int*) A->a;
    for (i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	assert(0);
	return NULL;
      }
      ia[irn[i]+1]++;
    }
    for (i = 0; i < m; i++) ia[i+1] += ia[i];
    for (i = 0; i < nz; i++){
      ai[ia[irn[i]]] = vali[i];
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  case MATRIX_TYPE_PATTERN:
    for (i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	assert(0);
	return NULL;
      }
      ia[irn[i]+1]++;
    }
    for (i = 0; i < m; i++) ia[i+1] += ia[i];
    for (i = 0; i < nz; i++){
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  case MATRIX_TYPE_UNKNOWN:
    for (i = 0; i < nz; i++){
      if (irn[i] < 0 || irn[i] >= m || jcn[i] < 0 || jcn[i] >= n) {
	assert(0);
	return NULL;
      }
      ia[irn[i]+1]++;
    }
    for (i = 0; i < m; i++) ia[i+1] += ia[i];
    memcpy(A->a, val0, A->size*((size_t)nz));
    for (i = 0; i < nz; i++){
      ja[ia[irn[i]]++] = jcn[i];
    }
    for (i = m; i > 0; i--) ia[i] = ia[i - 1];
    ia[0] = 0;
    break;
  default:
    assert(0);
    return NULL;
  }
  A->nz = nz;



  if(sum_repeated) A = SparseMatrix_sum_repeat_entries(A, sum_repeated);
 
  return A;
}


SparseMatrix SparseMatrix_from_coordinate_arrays(int nz, int m, int n, int *irn, int *jcn, void *val0, int type, size_t sz){
  return SparseMatrix_from_coordinate_arrays_internal(nz, m, n, irn, jcn, val0, type, sz, SUM_REPEATED_ALL);
}


SparseMatrix SparseMatrix_from_coordinate_arrays_not_compacted(int nz, int m, int n, int *irn, int *jcn, void *val0, int type, size_t sz){
  return SparseMatrix_from_coordinate_arrays_internal(nz, m, n, irn, jcn, val0, type, sz, SUM_REPEATED_NONE);
}

SparseMatrix SparseMatrix_add(SparseMatrix A, SparseMatrix B){
  int m, n;
  SparseMatrix C = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic, *jc;
  int i, j, nz, nzmax;

  assert(A && B);
  assert(A->format == B->format && A->format == FORMAT_CSR);/* other format not yet supported */
  assert(A->type == B->type);
  m = A->m;
  n = A->n;
  if (m != B->m || n != B->n) return NULL;

  nzmax = A->nz + B->nz;/* just assume that no entries overlaps for speed */

  C = SparseMatrix_new(m, n, nzmax, A->type, FORMAT_CSR);
  if (!C) goto RETURN;
  ic = C->ia;
  jc = C->ja;

  mask = MALLOC(sizeof(int)*((size_t) n));

  for (i = 0; i < n; i++) mask[i] = -1;

  nz = 0;
  ic[0] = 0;
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
    real *c = (real*) C->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = nz;
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
      ic[i+1] = nz;
    }
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    real *b = (real*) B->a;
    real *c = (real*) C->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = nz;
	jc[nz] = ja[j];
	c[2*nz] = a[2*j];
	c[2*nz+1] = a[2*j+1];
	nz++;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ic[i]){
	  jc[nz] = jb[j];
	  c[2*nz] = b[2*j];
	  c[2*nz+1] = b[2*j+1];
	  nz++;
	} else {
	  c[2*mask[jb[j]]] += b[2*j];
	  c[2*mask[jb[j]]+1] += b[2*j+1];
	}
      }
      ic[i+1] = nz;
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = (int*) A->a;
    int *b = (int*) B->a;
    int *c = (int*) C->a;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = nz;
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
      ic[i+1] = nz;
    }
    break;
  }
  case MATRIX_TYPE_PATTERN:{
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	mask[ja[j]] = nz;
	jc[nz] = ja[j];
	nz++;
      }
      for (j = ib[i]; j < ib[i+1]; j++){
	if (mask[jb[j]] < ic[i]){
	  jc[nz] = jb[j];
	  nz++;
	} 
      }
      ic[i+1] = nz;
    }
    break;
  }
  case MATRIX_TYPE_UNKNOWN:
    break;
  default:
    break;
  }
  C->nz = nz;

 RETURN:
  free(mask);

  return C;
}

static void SparseMatrix_multiply_dense1(SparseMatrix A, real *v, real **res, int dim){
  /* A v where v a dense matrix of second dimension dim. Real only for now. */
  int i, j, k, *ia, *ja, m;
  real *a, *u;

  assert(A->format == FORMAT_CSR);
  assert(A->type == MATRIX_TYPE_REAL);

  a = (real*) A->a;
  ia = A->ia;
  ja = A->ja;
  m = A->m;
  u = *res;

  if (!u) u = MALLOC(sizeof(real)*((size_t) m)*((size_t) dim));
  for (i = 0; i < m; i++){
    for (k = 0; k < dim; k++) u[i*dim+k] = 0.;
    for (j = ia[i]; j < ia[i+1]; j++){
      for (k = 0; k < dim; k++) u[i*dim+k] += a[j]*v[ja[j]*dim+k];
    }
  }

  *res = u;
}

void SparseMatrix_multiply_dense(SparseMatrix A, real *v, real **res, int dim){
  /* A * V, with A dimension m x n, with V of dimension n x dim. v[i*dim+j] gives V[i,j]. Result of dimension m x dim
 */

  SparseMatrix_multiply_dense1(A, v, res, dim);
}

void SparseMatrix_multiply_vector(SparseMatrix A, real *v, real **res, int transposed){
  /* A v or A^T v. Real only for now. */
  int i, j, *ia, *ja, n, m;
  real *a, *u = NULL;
  int *ai;
  assert(A->format == FORMAT_CSR);
  assert(A->type == MATRIX_TYPE_REAL || A->type == MATRIX_TYPE_INTEGER);

  ia = A->ia;
  ja = A->ja;
  m = A->m;
  n = A->n;
  u = *res;

  switch (A->type){
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    if (v){
      if (!transposed){
	if (!u) u = MALLOC(sizeof(real)*((size_t)m));
	for (i = 0; i < m; i++){
	  u[i] = 0.;
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[i] += a[j]*v[ja[j]];
	  }
	}
      } else {
	if (!u) u = MALLOC(sizeof(real)*((size_t)n));
	for (i = 0; i < n; i++) u[i] = 0.;
	for (i = 0; i < m; i++){
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[ja[j]] += a[j]*v[i];
	  }
	}
      }
    } else {
      /* v is assumed to be all 1's */
      if (!transposed){
	if (!u) u = MALLOC(sizeof(real)*((size_t)m));
	for (i = 0; i < m; i++){
	  u[i] = 0.;
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[i] += a[j];
	  }
	}
      } else {
	if (!u) u = MALLOC(sizeof(real)*((size_t)n));
	for (i = 0; i < n; i++) u[i] = 0.;
	for (i = 0; i < m; i++){
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[ja[j]] += a[j];
	  }
	}
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    if (v){
      if (!transposed){
	if (!u) u = MALLOC(sizeof(real)*((size_t)m));
	for (i = 0; i < m; i++){
	  u[i] = 0.;
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[i] += ai[j]*v[ja[j]];
	  }
	}
      } else {
	if (!u) u = MALLOC(sizeof(real)*((size_t)n));
	for (i = 0; i < n; i++) u[i] = 0.;
	for (i = 0; i < m; i++){
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[ja[j]] += ai[j]*v[i];
	  }
	}
      }
    } else {
      /* v is assumed to be all 1's */
      if (!transposed){
	if (!u) u = MALLOC(sizeof(real)*((size_t)m));
	for (i = 0; i < m; i++){
	  u[i] = 0.;
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[i] += ai[j];
	  }
	}
      } else {
	if (!u) u = MALLOC(sizeof(real)*((size_t)n));
	for (i = 0; i < n; i++) u[i] = 0.;
	for (i = 0; i < m; i++){
	  for (j = ia[i]; j < ia[i+1]; j++){
	    u[ja[j]] += ai[j];
	  }
	}
      }
    }
    break;
  default:
    assert(0);
    u = NULL;
  }
  *res = u;

}

SparseMatrix SparseMatrix_multiply(SparseMatrix A, SparseMatrix B){
  int m;
  SparseMatrix C = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic, *jc;
  int i, j, k, jj, type, nz;

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
  
  mask = MALLOC(sizeof(int)*((size_t)(B->n)));
  if (!mask) return NULL;

  for (i = 0; i < B->n; i++) mask[i] = -1;

  nz = 0;
  for (i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      for (k = ib[jj]; k < ib[jj+1]; k++){
	if (mask[jb[k]] != -i - 2){
	  if ((nz+1) <= nz) {
#ifdef DEBUG_PRINT
	    fprintf(stderr,"overflow in SparseMatrix_multiply !!!\n");
#endif
	    return NULL;
	  }
	  nz++;
	  mask[jb[k]] = -i - 2;
	}
      }
    }
  }

  C = SparseMatrix_new(m, B->n, nz, type, FORMAT_CSR);
  if (!C) goto RETURN;
  ic = C->ia;
  jc = C->ja;
  
  nz = 0;

  switch (type){
  case MATRIX_TYPE_REAL:
    {
      real *a = (real*) A->a;
      real *b = (real*) B->a;
      real *c = (real*) C->a;
      ic[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (k = ib[jj]; k < ib[jj+1]; k++){
	    if (mask[jb[k]] < ic[i]){
	      mask[jb[k]] = nz;
	      jc[nz] = jb[k];
	      c[nz] = a[j]*b[k];
	      nz++;
	    } else {
	      assert(jc[mask[jb[k]]] == jb[k]);
	      c[mask[jb[k]]] += a[j]*b[k];
	    }
	  }
	}
	ic[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    {
      real *a = (real*) A->a;
      real *b = (real*) B->a;
      real *c = (real*) C->a;
      a = (real*) A->a;
      b = (real*) B->a;
      c = (real*) C->a;
      ic[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (k = ib[jj]; k < ib[jj+1]; k++){
	    if (mask[jb[k]] < ic[i]){
	      mask[jb[k]] = nz;
	      jc[nz] = jb[k];
	      c[2*nz] = a[2*j]*b[2*k] - a[2*j+1]*b[2*k+1];/*real part */
	      c[2*nz+1] = a[2*j]*b[2*k+1] + a[2*j+1]*b[2*k];/*img part */
	      nz++;
	    } else {
	      assert(jc[mask[jb[k]]] == jb[k]);
	      c[2*mask[jb[k]]] += a[2*j]*b[2*k] - a[2*j+1]*b[2*k+1];/*real part */
	      c[2*mask[jb[k]]+1] += a[2*j]*b[2*k+1] + a[2*j+1]*b[2*k];/*img part */
	    }
	  }
	}
	ic[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    {
      int *a = (int*) A->a;
      int *b = (int*) B->a;
      int *c = (int*) C->a;
      ic[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (k = ib[jj]; k < ib[jj+1]; k++){
	    if (mask[jb[k]] < ic[i]){
	      mask[jb[k]] = nz;
	      jc[nz] = jb[k];
	      c[nz] = a[j]*b[k];
	      nz++;
	    } else {
	      assert(jc[mask[jb[k]]] == jb[k]);
	      c[mask[jb[k]]] += a[j]*b[k];
	    }
	  }
	}
	ic[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    ic[0] = 0;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jj = ja[j];
	for (k = ib[jj]; k < ib[jj+1]; k++){
	  if (mask[jb[k]] < ic[i]){
	    mask[jb[k]] = nz;
	    jc[nz] = jb[k];
	    nz++;
	  } else {
	    assert(jc[mask[jb[k]]] == jb[k]);
	  }
	}
      }
      ic[i+1] = nz;
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
  default:
    SparseMatrix_delete(C);
    C = NULL; goto RETURN;
    break;
  }
  
  C->nz = nz;

 RETURN:
  free(mask);
  return C;
}



SparseMatrix SparseMatrix_multiply3(SparseMatrix A, SparseMatrix B, SparseMatrix C){
  int m;
  SparseMatrix D = NULL;
  int *mask = NULL;
  int *ia = A->ia, *ja = A->ja, *ib = B->ia, *jb = B->ja, *ic = C->ia, *jc = C->ja, *id, *jd;
  int i, j, k, l, ll, jj, type, nz;

  assert(A->format == B->format && A->format == FORMAT_CSR);/* other format not yet supported */

  m = A->m;
  if (A->n != B->m) return NULL;
  if (B->n != C->m) return NULL;

  if (A->type != B->type || B->type != C->type){
#ifdef DEBUG
    printf("in SparseMatrix_multiply, the matrix types do not match, right now only multiplication of matrices of the same type is supported\n");
#endif
    return NULL;
  }
  type = A->type;
  
  mask = MALLOC(sizeof(int)*((size_t)(C->n)));
  if (!mask) return NULL;

  for (i = 0; i < C->n; i++) mask[i] = -1;

  nz = 0;
  for (i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      for (l = ib[jj]; l < ib[jj+1]; l++){
	ll = jb[l];
	for (k = ic[ll]; k < ic[ll+1]; k++){
	  if (mask[jc[k]] != -i - 2){
	    if ((nz+1) <= nz) {
#ifdef DEBUG_PRINT
	      fprintf(stderr,"overflow in SparseMatrix_multiply !!!\n");
#endif
	      return NULL;
	    }
	    nz++;
	    mask[jc[k]] = -i - 2;
	  }
	}
      }
    }
  }

  D = SparseMatrix_new(m, C->n, nz, type, FORMAT_CSR);
  if (!D) goto RETURN;
  id = D->ia;
  jd = D->ja;
  
  nz = 0;

  switch (type){
  case MATRIX_TYPE_REAL:
    {
      real *a = (real*) A->a;
      real *b = (real*) B->a;
      real *c = (real*) C->a;
      real *d = (real*) D->a;
      id[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (l = ib[jj]; l < ib[jj+1]; l++){
	    ll = jb[l];
	    for (k = ic[ll]; k < ic[ll+1]; k++){
	      if (mask[jc[k]] < id[i]){
		mask[jc[k]] = nz;
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
	id[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    {
      real *a = (real*) A->a;
      real *b = (real*) B->a;
      real *c = (real*) C->a;
      real *d = (real*) D->a;
      id[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (l = ib[jj]; l < ib[jj+1]; l++){
	    ll = jb[l];
	    for (k = ic[ll]; k < ic[ll+1]; k++){
	      if (mask[jc[k]] < id[i]){
		mask[jc[k]] = nz;
		jd[nz] = jc[k];
		d[2*nz] = (a[2*j]*b[2*l] - a[2*j+1]*b[2*l+1])*c[2*k] 
		  - (a[2*j]*b[2*l+1] + a[2*j+1]*b[2*l])*c[2*k+1];/*real part */
		d[2*nz+1] = (a[2*j]*b[2*l+1] + a[2*j+1]*b[2*l])*c[2*k]
		  + (a[2*j]*b[2*l] - a[2*j+1]*b[2*l+1])*c[2*k+1];/*img part */
		nz++;
	      } else {
		assert(jd[mask[jc[k]]] == jc[k]);
		d[2*mask[jc[k]]] += (a[2*j]*b[2*l] - a[2*j+1]*b[2*l+1])*c[2*k] 
		  - (a[2*j]*b[2*l+1] + a[2*j+1]*b[2*l])*c[2*k+1];/*real part */
		d[2*mask[jc[k]]+1] += (a[2*j]*b[2*l+1] + a[2*j+1]*b[2*l])*c[2*k]
		  + (a[2*j]*b[2*l] - a[2*j+1]*b[2*l+1])*c[2*k+1];/*img part */
	      }
	    }
	  }
	}
	id[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    {
      int *a = (int*) A->a;
      int *b = (int*) B->a;
      int *c = (int*) C->a;
      int *d = (int*) D->a;
      id[0] = 0;
      for (i = 0; i < m; i++){
	for (j = ia[i]; j < ia[i+1]; j++){
	  jj = ja[j];
	  for (l = ib[jj]; l < ib[jj+1]; l++){
	    ll = jb[l];
	    for (k = ic[ll]; k < ic[ll+1]; k++){
	      if (mask[jc[k]] < id[i]){
		mask[jc[k]] = nz;
		jd[nz] = jc[k];
		d[nz] += a[j]*b[l]*c[k];
		nz++;
	      } else {
		assert(jd[mask[jc[k]]] == jc[k]);
		d[mask[jc[k]]] += a[j]*b[l]*c[k];
	      }
	    }
	  }
	}
	id[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    id[0] = 0;
    for (i = 0; i < m; i++){
      for (j = ia[i]; j < ia[i+1]; j++){
	jj = ja[j];
	for (l = ib[jj]; l < ib[jj+1]; l++){
	  ll = jb[l];
	  for (k = ic[ll]; k < ic[ll+1]; k++){
	    if (mask[jc[k]] < id[i]){
	      mask[jc[k]] = nz;
	      jd[nz] = jc[k];
	      nz++;
	    } else {
	      assert(jd[mask[jc[k]]] == jc[k]);
	    }
	  }
	}
      }
      id[i+1] = nz;
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
  default:
    SparseMatrix_delete(D);
    D = NULL; goto RETURN;
    break;
  }
  
  D->nz = nz;

 RETURN:
  free(mask);
  return D;
}

SparseMatrix SparseMatrix_sum_repeat_entries(SparseMatrix A, int what_to_sum){
  /* sum repeated entries in the same row, i.e., {1,1}->1, {1,1}->2 becomes {1,1}->3 */
  int *ia = A->ia, *ja = A->ja, type = A->type, n = A->n;
  int *mask = NULL, nz = 0, i, j, sta;

  if (what_to_sum == SUM_REPEATED_NONE) return A;

  mask = MALLOC(sizeof(int)*((size_t)n));
  for (i = 0; i < n; i++) mask[i] = -1;

  switch (type){
  case MATRIX_TYPE_REAL:
    {
      real *a = (real*) A->a;
      nz = 0;
      sta = ia[0];
      for (i = 0; i < A->m; i++){
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    a[nz] = a[j];
	    mask[ja[j]] = nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	    a[mask[ja[j]]] += a[j];
	  }
	}
	sta = ia[i+1];
	ia[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_COMPLEX:
    {
      real *a = (real*) A->a;
      if (what_to_sum == SUM_REPEATED_ALL){
	nz = 0;
	sta = ia[0];
	for (i = 0; i < A->m; i++){
	  for (j = sta; j < ia[i+1]; j++){
	    if (mask[ja[j]] < ia[i]){
	      ja[nz] = ja[j];
	      a[2*nz] = a[2*j];
	      a[2*nz+1] = a[2*j+1];
	      mask[ja[j]] = nz++;
	    } else {
	      assert(ja[mask[ja[j]]] == ja[j]);
	      a[2*mask[ja[j]]] += a[2*j];
	      a[2*mask[ja[j]]+1] += a[2*j+1];
	    }
	  }
	  sta = ia[i+1];
	  ia[i+1] = nz;
	}
      }
    }
    break;
  case MATRIX_TYPE_INTEGER:
    {
      int *a = (int*) A->a;
      nz = 0;
      sta = ia[0];
      for (i = 0; i < A->m; i++){
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    a[nz] = a[j];
	    mask[ja[j]] = nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	    a[mask[ja[j]]] += a[j];
	  }
	}
	sta = ia[i+1];
	ia[i+1] = nz;	
      }
    }
    break;
  case MATRIX_TYPE_PATTERN:
    {
      nz = 0;
      sta = ia[0];
      for (i = 0; i < A->m; i++){
	for (j = sta; j < ia[i+1]; j++){
	  if (mask[ja[j]] < ia[i]){
	    ja[nz] = ja[j];
	    mask[ja[j]] = nz++;
	  } else {
	    assert(ja[mask[ja[j]]] == ja[j]);
	  }
	}
	sta = ia[i+1];
	ia[i+1] = nz;
      }
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
    return NULL;
    break;
  default:
    return NULL;
    break;
  }
  A->nz = nz;
  free(mask);
  return A;
}

SparseMatrix SparseMatrix_coordinate_form_add_entries(SparseMatrix A, int nentries, int *irn, int *jcn, void *val){
  int nz, nzmax, i;
  
  assert(A->format == FORMAT_COORD);
  if (nentries <= 0) return A;
  nz = A->nz;

  if (nz + nentries >= A->nzmax){
    nzmax = nz + nentries;
     nzmax = MAX(10, (int) 0.2*nzmax) + nzmax;
    A = SparseMatrix_realloc(A, nzmax);
  }
  memcpy((char*) A->ia + ((size_t)nz)*sizeof(int)/sizeof(char), irn, sizeof(int)*((size_t)nentries));
  memcpy((char*) A->ja + ((size_t)nz)*sizeof(int)/sizeof(char), jcn, sizeof(int)*((size_t)nentries));
  if (A->size) memcpy((char*) A->a + ((size_t)nz)*A->size/sizeof(char), val, A->size*((size_t)nentries));
  for (i = 0; i < nentries; i++) {
    if (irn[i] >= A->m) A->m = irn[i]+1;
    if (jcn[i] >= A->n) A->n = jcn[i]+1;
  }
  A->nz += nentries;
  return A;
}


SparseMatrix SparseMatrix_remove_diagonal(SparseMatrix A){
  int i, j, *ia, *ja, nz, sta;

  if (!A) return A;

  nz = 0;
  ia = A->ia;
  ja = A->ja;
  sta = ia[0];
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz] = ja[j];
	  a[2*nz] = a[2*j];
	  a[2*nz+1] = a[2*j+1];
	  nz++;
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = (int*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] != i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
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
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_UNKNOWN:
    return NULL;
  default:
    return NULL;
  }

  return A;
}


SparseMatrix SparseMatrix_remove_upper(SparseMatrix A){/* remove diag and upper diag */
  int i, j, *ia, *ja, nz, sta;

  if (!A) return A;

  nz = 0;
  ia = A->ia;
  ja = A->ja;
  sta = ia[0];
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz] = ja[j];
	  a[2*nz] = a[2*j];
	  a[2*nz+1] = a[2*j+1];
	  nz++;
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = (int*) A->a;
    for (i = 0; i < A->m; i++){
      for (j = sta; j < ia[i+1]; j++){
	if (ja[j] < i){
	  ja[nz] = ja[j];
	  a[nz++] = a[j];
	}
      }
      sta = ia[i+1];
      ia[i+1] = nz;
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
      ia[i+1] = nz;
    }
    A->nz = nz;
    break;
  }
  case MATRIX_TYPE_UNKNOWN:
    return NULL;
  default:
    return NULL;
  }

  clear_flag(A->property, MATRIX_PATTERN_SYMMETRIC);
  clear_flag(A->property, MATRIX_SYMMETRIC);
  clear_flag(A->property, MATRIX_SKEW);
  clear_flag(A->property, MATRIX_HERMITIAN);
  return A;
}




SparseMatrix SparseMatrix_divide_row_by_degree(SparseMatrix A){
  int i, j, *ia, *ja;
  real deg;

  if (!A) return A;

  ia = A->ia;
  ja = A->ja;
  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      deg = ia[i+1] - ia[i];
      for (j = ia[i]; j < ia[i+1]; j++){
	a[j] = a[j]/deg;
      }
    }
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    for (i = 0; i < A->m; i++){
      deg = ia[i+1] - ia[i];
      for (j = ia[i]; j < ia[i+1]; j++){
	if (ja[j] != i){
	  a[2*j] = a[2*j]/deg;
	  a[2*j+1] = a[2*j+1]/deg;
	}
      }
    }
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    assert(0);/* this operation would not make sense for int matrix */
    break;
  }
  case MATRIX_TYPE_PATTERN:{
    break;
  }
  case MATRIX_TYPE_UNKNOWN:
    return NULL;
  default:
    return NULL;
  }

  return A;
}


SparseMatrix SparseMatrix_get_real_adjacency_matrix_symmetrized(SparseMatrix A){
  /* symmetric, all entries to 1, diaginal removed */
  int i, *ia, *ja, nz, m, n;
  real *a;
  SparseMatrix B;

  if (!A) return A;
  
  nz = A->nz;
  ia = A->ia;
  ja = A->ja;
  n = A->n;
  m = A->m;

  if (n != m) return NULL;

  B = SparseMatrix_new(m, n, nz, MATRIX_TYPE_PATTERN, FORMAT_CSR);

  memcpy(B->ia, ia, sizeof(int)*((size_t)(m+1)));
  memcpy(B->ja, ja, sizeof(int)*((size_t)nz));
  B->nz = A->nz;

  A = SparseMatrix_symmetrize(B, TRUE);
  SparseMatrix_delete(B);
  A = SparseMatrix_remove_diagonal(A);
  A->a = MALLOC(sizeof(real)*((size_t)(A->nz)));
  a = (real*) A->a;
  for (i = 0; i < A->nz; i++) a[i] = 1.;
  A->type = MATRIX_TYPE_REAL;
  A->size = sizeof(real);
  return A;
}

SparseMatrix SparseMatrix_apply_fun(SparseMatrix A, double (*fun)(double x)){
  int i, j;
  real *a;


  if (!A) return A;
  if (A->format != FORMAT_CSR && A->type != MATRIX_TYPE_REAL) {
#ifdef DEBUG
    printf("only CSR and real matrix supported.\n");
#endif
    return A;
  }


  a = (real*) A->a;
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
  if (A->a) memcpy(B->a, A->a, A->size*((size_t)A->nz));
  B->property = A->property;
  B->nz = A->nz;
  return B;
}

int SparseMatrix_has_diagonal(SparseMatrix A){

  int i, j, m = A->m, *ia = A->ia, *ja = A->ja;

  for (i = 0; i < m; i++){
    for (j = ia[i]; j < ia[i+1]; j++){
      if (i == ja[j]) return TRUE;
    }
  }
  return FALSE;
}

static void SparseMatrix_level_sets_internal(int khops, SparseMatrix A, int root, int *nlevel, int **levelset_ptr, int **levelset, int **mask, int reinitialize_mask){
  /* mask is assumed to be initialized to negative if provided.
     . On exit, mask = levels for visited nodes (1 for root, 2 for its neighbors, etc), 
     . unless reinitialize_mask = TRUE, in which case mask = -1.
     khops: max number of hops allowed. If khops < 0, no limit is applied.
     A: the graph, undirected
     root: starting node
     nlevel: max distance to root from any node (in the connected comp)
     levelset_ptr, levelset: the level sets
   */
  int i, j, sta = 0, sto = 1, nz, ii;
  int m = A->m, *ia = A->ia, *ja = A->ja;

  if (!(*levelset_ptr)) *levelset_ptr = MALLOC(sizeof(int)*((size_t)(m+2)));
  if (!(*levelset)) *levelset = MALLOC(sizeof(int)*((size_t)m));
  if (!(*mask)) {
    *mask = malloc(sizeof(int)*((size_t)m));
    for (i = 0; i < m; i++) (*mask)[i] = UNMASKED;
  }

  *nlevel = 0;
  assert(root >= 0 && root < m);
  (*levelset_ptr)[0] = 0;
  (*levelset_ptr)[1] = 1;
  (*levelset)[0] = root;
  (*mask)[root] = 1;
  *nlevel = 1;
  nz = 1;
  sta = 0; sto = 1;
  while (sto > sta && (khops < 0 || *nlevel <= khops)){
    for (i = sta; i < sto; i++){
      ii = (*levelset)[i];
      for (j = ia[ii]; j < ia[ii+1]; j++){
	if (ii == ja[j]) continue;
	if ((*mask)[ja[j]] < 0){
	  (*levelset)[nz++] = ja[j];
	  (*mask)[ja[j]] = *nlevel + 1;
	}
      }
    }
    (*levelset_ptr)[++(*nlevel)] = nz;
    sta = sto;
    sto = nz;
  }
  if (khops < 0 || *nlevel <= khops){
    (*nlevel)--;
  }
  if (reinitialize_mask) for (i = 0; i < (*levelset_ptr)[*nlevel]; i++) (*mask)[(*levelset)[i]] = UNMASKED;
}

void SparseMatrix_level_sets(SparseMatrix A, int root, int *nlevel, int **levelset_ptr, int **levelset, int **mask, int reinitialize_mask){

  int khops = -1;

  return SparseMatrix_level_sets_internal(khops, A, root, nlevel, levelset_ptr, levelset, mask, reinitialize_mask);

}
void SparseMatrix_level_sets_khops(int khops, SparseMatrix A, int root, int *nlevel, int **levelset_ptr, int **levelset, int **mask, int reinitialize_mask){

  return SparseMatrix_level_sets_internal(khops, A, root, nlevel, levelset_ptr, levelset, mask, reinitialize_mask);

}

void SparseMatrix_weakly_connected_components(SparseMatrix A0, int *ncomp, int **comps, int **comps_ptr){
  SparseMatrix A = A0;
  int *levelset_ptr = NULL, *levelset = NULL, *mask = NULL, nlevel;
  int m = A->m, i, nn;

  if (!SparseMatrix_is_symmetric(A, TRUE)){
    A = SparseMatrix_symmetrize(A, TRUE);
  }
  if (!(*comps_ptr)) *comps_ptr = MALLOC(sizeof(int)*((size_t)(m+1)));

  *ncomp = 0;
  (*comps_ptr)[0] = 0;
  for (i = 0; i < m; i++){
    if (i == 0 || mask[i] < 0) {
      SparseMatrix_level_sets(A, i, &nlevel, &levelset_ptr, &levelset, &mask, FALSE);
      if (i == 0) *comps = levelset;
      nn = levelset_ptr[nlevel];
      levelset += nn;
      (*comps_ptr)[(*ncomp)+1] = (*comps_ptr)[(*ncomp)] + nn;
      (*ncomp)++;
    }
    
  }
  if (A != A0) SparseMatrix_delete(A);
  free(levelset_ptr);

  free(mask);
}



struct nodedata_struct {
  real dist;/* distance to root */
  int id;/*node id */
};
typedef struct nodedata_struct* nodedata;

static int cmp(void*i, void*j){
  nodedata d1, d2;

  d1 = i;
  d2 = j;
  if (d1->dist > d2->dist){
    return 1;
  }
  if (d1->dist < d2->dist) {
    return -1;
  }
  return 0;
}

static int Dijkstra_internal(SparseMatrix A, int root, real *dist, int *nlist, int *list, real *dist_max, int *mask){
  /* Find the shortest path distance of all nodes to root. If khops >= 0, the shortest ath is of distance <= khops,

     A: the nxn connectivity matrix. Entries are assumed to be nonnegative. Absolute value will be taken if 
     .  entry value is negative.
     dist: length n. On on exit contain the distance from root to every other node. dist[root] = 0. dist[i] = distance from root to node i.
     .     if the graph is disconnected, unreachable node have a distance -1.
     .     note: ||root - list[i]|| =!= dist[i] !!!, instead, ||root - list[i]|| == dist[list[i]]
     nlist: number of nodes visited
     list: length n. the list of node in order of their extraction from the heap. 
     .     The distance from root to last in the list should be the maximum
     dist_max: the maximum distance, should be realized at node list[nlist-1].
     mask: if NULL, not used. Otherwise, only nodes i with mask[i] > 0 will be considered
     return: 0 if every node is reachable. -1 if not */

  int m = A->m, i, j, jj, *ia = A->ia, *ja = A->ja, heap_id;
  BinaryHeap h;
  real *a = NULL, *aa;
  int *ai;
  nodedata ndata, ndata_min;
  enum {UNVISITED = -2, FINISHED = -1};
  int *heap_ids; /* node ID to heap ID array. Initialised to UNVISITED. 
		   Set to FINISHED after extracted as min from heap */
  int found = 0;

  assert(SparseMatrix_is_symmetric(A, TRUE));

  assert(m == A->n);

  switch (A->type){
  case MATRIX_TYPE_COMPLEX:
    aa = (real*) A->a;
    a = MALLOC(sizeof(real)*((size_t)(A->nz)));
    for (i = 0; i < A->nz; i++) a[i] = aa[i*2];
    break;
  case MATRIX_TYPE_REAL:
    a = (real*) A->a;
    break;
  case MATRIX_TYPE_INTEGER:
    ai = (int*) A->a;
    a = MALLOC(sizeof(real)*((size_t)(A->nz)));
    for (i = 0; i < A->nz; i++) a[i] = (real) ai[i];
    break;
  case MATRIX_TYPE_PATTERN:
    a = MALLOC(sizeof(real)*((size_t)A->nz));
    for (i = 0; i < A->nz; i++) a[i] = 1.;
    break;
  default:
    assert(0);/* no such matrix type */
  }

  heap_ids = MALLOC(sizeof(int)*((size_t)m));
  for (i = 0; i < m; i++) {
    dist[i] = -1;
    heap_ids[i] = UNVISITED;
  }

  h = BinaryHeap_new(cmp);
  assert(h);

  /* add root as the first item in the heap */
  ndata = MALLOC(sizeof(struct nodedata_struct));
  ndata->dist = 0;
  ndata->id = root;
  heap_ids[root] = BinaryHeap_insert(h, ndata);
  
  assert(heap_ids[root] >= 0);/* by design heap ID from BinaryHeap_insert >=0*/

  while ((ndata_min = BinaryHeap_extract_min(h))){
    i = ndata_min->id;
    dist[i] = ndata_min->dist;
    list[found++] = i;
    heap_ids[i] = FINISHED;
    //fprintf(stderr," =================\n min extracted is id=%d, dist=%f\n",i, ndata_min->dist);
    for (j = ia[i]; j < ia[i+1]; j++){
      jj = ja[j];
      heap_id = heap_ids[jj];

      if (jj == i || heap_id == FINISHED || (mask && mask[jj] < 0)) continue;

      if (heap_id == UNVISITED){
	ndata = MALLOC(sizeof(struct nodedata_struct));
	ndata->dist = fabs(a[j]) + ndata_min->dist;
	ndata->id = jj;
	heap_ids[jj] = BinaryHeap_insert(h, ndata);
	//fprintf(stderr," set neighbor id=%d, dist=%f, hid = %d, a[%d]=%f, dist=%f\n",jj, ndata->dist, heap_ids[jj], jj, a[j], ndata->dist);

      } else {
	ndata = BinaryHeap_get_item(h, heap_id);
	ndata->dist = MIN(ndata->dist, fabs(a[j]) + ndata_min->dist);
	assert(ndata->id == jj);
	BinaryHeap_reset(h, heap_id, ndata); 
	//fprintf(stderr," reset neighbor id=%d, dist=%f, hid = %d, a[%d]=%f, dist=%f\n",jj, ndata->dist,heap_id, jj, a[j], ndata->dist);
     }
    }
    free(ndata_min);
  }
  *nlist = found;
  *dist_max = dist[i];


  BinaryHeap_delete(h, free);
  free(heap_ids);
  if (a && a != A->a) free(a);
  if (found == m || mask){
    return 0;
  } else {
    return -1;
  }
}

static int Dijkstra(SparseMatrix A, int root, real *dist, int *nlist, int *list, real *dist_max){
  return Dijkstra_internal(A, root, dist, nlist, list, dist_max, NULL);
}

static int Dijkstra_masked(SparseMatrix A, int root, real *dist, int *nlist, int *list, real *dist_max, int *mask){
  /* this makes the algorithm only consider nodes that are masked.
     nodes are masked as 1, 2, ..., mask_max, which is (the number of hops from root)+1.
     Only paths consists of nodes that are masked are allowed. */
     
  return Dijkstra_internal(A, root, dist, nlist, list, dist_max, mask);
}

void SparseMatrix_decompose_to_supervariables(SparseMatrix A, int *ncluster, int **cluster, int **clusterp){
  /* nodes for a super variable if they share exactly the same neighbors. This is know as modules in graph theory.
     We work on columns only and columns with the same pattern are grouped as a super variable
   */
  int *ia = A->ia, *ja = A->ja, n = A->n, m = A->m;
  int *super = NULL, *nsuper = NULL, i, j, *mask = NULL, isup, *newmap, isuper;

  super = MALLOC(sizeof(int)*((size_t)(n)));
  nsuper = MALLOC(sizeof(int)*((size_t)(n+1)));
  mask = MALLOC(sizeof(int)*((size_t)n));
  newmap = MALLOC(sizeof(int)*((size_t)n));
  nsuper++;

  isup = 0;
  for (i = 0; i < n; i++) super[i] = isup;/* every node belongs to super variable 0 by default */
  nsuper[0] = n;
  for (i = 0; i < n; i++) mask[i] = -1;
  isup++;

  for (i = 0; i < m; i++){
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
  for (i = 0; i < n; i++){
    printf("node %d is in supernode %d\n",i, super[i]);
  }
#endif
#ifdef PRINT
  fprintf(stderr, "n = %d, nsup = %d\n",n,isup);
#endif
  /* now accumulate super nodes */
  nsuper--;
  nsuper[0] = 0;
  for (i = 0; i < isup; i++) nsuper[i+1] += nsuper[i];

  *cluster = newmap;
  for (i = 0; i < n; i++){
    isuper = super[i];
    (*cluster)[nsuper[isuper]++] = i;
  }
  for (i = isup; i > 0; i--) nsuper[i] = nsuper[i-1];
  nsuper[0] = 0;
  *clusterp = nsuper;
  *ncluster = isup;

#ifdef PRINT
  for (i = 0; i < *ncluster; i++){
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
  int nz = A->nz, type = A->type;
  int m = A->m, n = A->n, i, j;
  SparseMatrix B = NULL;
  if (!A) return NULL;
  if (nz > 0){
    irn = MALLOC(sizeof(int)*((size_t)nz)*2);
    jcn = MALLOC(sizeof(int)*((size_t)nz)*2);
  }

  if (A->a){
    assert(A->size != 0 && nz > 0);
    val = MALLOC(A->size*2*((size_t)nz));
    memcpy(val, A->a, A->size*((size_t)nz));
    memcpy(((char*) val) + ((size_t)nz)*A->size, A->a, A->size*((size_t)nz));
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
  SparseMatrix_set_symmetric(B);
  SparseMatrix_set_pattern_symmetric(B);
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
    if (A->m == A->n && SparseMatrix_is_symmetric(A, TRUE)) return A;
    break;
  case BIPARTITE_UNSYM:
    if (A->m == A->n && SparseMatrix_is_symmetric(A, FALSE)) return A;
    break;
  case BIPARTITE_ALWAYS:
    break;
  default:
    assert(0);
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
  int nz = 0, i, j, *irn, *jcn, *ia = A->ia, *ja = A->ja, m = A->m, n = A->n;
  int *cmask, *rmask;
  void *v = NULL;
  SparseMatrix B = NULL;
  int irow = 0, icol = 0;

  if (nrow <= 0 || ncol <= 0) return NULL;

  

  rmask = MALLOC(sizeof(int)*((size_t)m));
  cmask = MALLOC(sizeof(int)*((size_t)n));
  for (i = 0; i < m; i++) rmask[i] = -1;
  for (i = 0; i < n; i++) cmask[i] = -1;

  if (rindices){
    for (i = 0; i < nrow; i++) {
      if (rindices[i] >= 0 && rindices[i] < m){
	rmask[rindices[i]] = irow++;
      }
    }
  } else {
    for (i = 0; i < nrow; i++) {
      rmask[i] = irow++;
    }
  }

  if (cindices){
    for (i = 0; i < ncol; i++) {
      if (cindices[i] >= 0 && cindices[i] < n){
	cmask[cindices[i]] = icol++;
      }
    }
  } else {
    for (i = 0; i < ncol; i++) {
      cmask[i] = icol++;
    }
  }

  for (i = 0; i < m; i++){
    if (rmask[i] < 0) continue;
    for (j = ia[i]; j < ia[i+1]; j++){
      if (cmask[ja[j]] < 0) continue;
      nz++;
    }
  }


  switch (A->type){
  case MATRIX_TYPE_REAL:{
    real *a = (real*) A->a;
    real *val;
    irn = MALLOC(sizeof(int)*((size_t)nz));
    jcn = MALLOC(sizeof(int)*((size_t)nz));
    val = MALLOC(sizeof(real)*((size_t)nz));

    nz = 0;
    for (i = 0; i < m; i++){
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz] = cmask[ja[j]];
	val[nz++] = a[j];
      }
    }
    v = (void*) val;
    break;
  }
  case MATRIX_TYPE_COMPLEX:{
    real *a = (real*) A->a;
    real *val;

    irn = MALLOC(sizeof(int)*((size_t)nz));
    jcn = MALLOC(sizeof(int)*((size_t)nz));
    val = MALLOC(sizeof(real)*2*((size_t)nz));

    nz = 0;
    for (i = 0; i < m; i++){
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz] = cmask[ja[j]];
	val[2*nz] = a[2*j];
	val[2*nz+1] = a[2*j+1];
	nz++;
      }
    }
    v = (void*) val;
    break;
  }
  case MATRIX_TYPE_INTEGER:{
    int *a = (int*) A->a;
    int *val;

    irn = MALLOC(sizeof(int)*((size_t)nz));
    jcn = MALLOC(sizeof(int)*((size_t)nz));
    val = MALLOC(sizeof(int)*((size_t)nz));

    nz = 0;
    for (i = 0; i < m; i++){
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz] = cmask[ja[j]];
	val[nz] = a[j];
	nz++;
      }
    }
    v = (void*) val;
    break;
  }
  case MATRIX_TYPE_PATTERN:
    irn = MALLOC(sizeof(int)*((size_t)nz));
    jcn = MALLOC(sizeof(int)*((size_t)nz));
    nz = 0;
     for (i = 0; i < m; i++){
      if (rmask[i] < 0) continue;
      for (j = ia[i]; j < ia[i+1]; j++){
	if (cmask[ja[j]] < 0) continue;
	irn[nz] = rmask[i];
	jcn[nz++] = cmask[ja[j]];
      }
    }
    break;
  case MATRIX_TYPE_UNKNOWN:
    free(rmask);
    free(cmask);
    return NULL;
  default:
    free(rmask);
    free(cmask);
    return NULL;
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
  real *a;
  int i;

  free(A->a);
  A->a = MALLOC(sizeof(real)*((size_t)A->nz));
  a = (real*) (A->a);
  for (i = 0; i < A->nz; i++) a[i] = 1.;
  A->type = MATRIX_TYPE_REAL;
  A->size = sizeof(real);
  return A;

}

SparseMatrix SparseMatrix_from_dense(int m, int n, real *x){
  /* wrap a mxn matrix into a sparse matrix. the {i,j} entry of the matrix is in x[i*n+j], 0<=i<m; 0<=j<n */
  int i, j, *ja;
  real *a;
  SparseMatrix A = SparseMatrix_new(m, n, m*n, MATRIX_TYPE_REAL, FORMAT_CSR);

  A->ia[0] = 0;
  for (i = 1; i <= m; i++) (A->ia)[i] = (A->ia)[i-1] + n;
  
  ja = A->ja;
  a = (real*) A->a;
  for (i = 0; i < m; i++){
    for (j = 0; j < n; j++) {
      ja[j] = j;
      a[j] = x[i*n+j];
    }
    ja += n; a += j;
  }
  A->nz = m*n;
  return A;

}


int SparseMatrix_distance_matrix(SparseMatrix D0, int weighted, real **dist0){
  /*
    Input:
    D: the graph. If weighted, the entry values is used.
    weighted: whether to treat the graph as weighted
    Output:
    dist: of dimension nxn, dist[i*n+j] gives the distance of node i to j.
    return: flag. if not zero, the graph is not connected, or out of memory.
  */
  SparseMatrix D = D0;
  int m = D->m, n = D->n;
  int *levelset_ptr = NULL, *levelset = NULL, *mask = NULL;
  real *dist = NULL;
  int nlist, *list = NULL;
  int flag = 0, i, j, k, nlevel;
  real dmax;

  if (!SparseMatrix_is_symmetric(D, FALSE)){
    D = SparseMatrix_symmetrize(D, FALSE);
  }

  assert(m == n);

  if (!(*dist0)) *dist0 = MALLOC(sizeof(real)*n*n);
  for (i = 0; i < n*n; i++) (*dist0)[i] = -1;

  if (!weighted){
    for (k = 0; k < n; k++){
      SparseMatrix_level_sets(D, k, &nlevel, &levelset_ptr, &levelset, &mask, TRUE);
      assert(levelset_ptr[nlevel] == n);
      for (i = 0; i < nlevel; i++) {
	for (j = levelset_ptr[i]; j < levelset_ptr[i+1]; j++){
	  (*dist0)[k*n+levelset[j]] = i;
	}
      }
     }
 } else {
    list = MALLOC(sizeof(int)*n);
    for (k = 0; k < n; k++){
      dist = &((*dist0)[k*n]);
      flag = Dijkstra(D, k, dist, &nlist, list, &dmax);
    }
  }

  free(levelset_ptr);
  free(levelset);
  free(mask);
  
  if (D != D0) SparseMatrix_delete(D);
  free(list);
  return flag;
}

SparseMatrix SparseMatrix_distance_matrix_khops(int khops, SparseMatrix D0, int weighted){
  /*
    Input:
    khops: number of hops allow. If khops < 0, this will give a dense distances. Otherwise it gives a sparse matrix that represent the k-neighborhood graph
    D: the graph. If weighted, the entry values is used.
    weighted: whether to treat the graph as weighted
    Output:
    DD: of dimension nxn. DD[i,j] gives the shortest path distance, subject to the fact that the short oath must be of <= khops.
    return: flag. if not zero, the graph is not connected, or out of memory.
  */
  SparseMatrix D = D0, B, C;
  int m = D->m, n = D->n;
  int *levelset_ptr = NULL, *levelset = NULL, *mask = NULL;
  real *dist = NULL;
  int nlist, *list = NULL;
  int flag = 0, i, j, k, itmp, nlevel;
  real dmax, dtmp;

  if (!SparseMatrix_is_symmetric(D, FALSE)){
    D = SparseMatrix_symmetrize(D, FALSE);
  }

  assert(m == n);
 
  B = SparseMatrix_new(n, n, 1, MATRIX_TYPE_REAL, FORMAT_COORD);

  if (!weighted){
    for (k = 0; k < n; k++){
      SparseMatrix_level_sets_khops(khops, D, k, &nlevel, &levelset_ptr, &levelset, &mask, TRUE);
      for (i = 0; i < nlevel; i++) {
	for (j = levelset_ptr[i]; j < levelset_ptr[i+1]; j++){
	  itmp = levelset[j]; dtmp = i;
	  if (k != itmp) B = SparseMatrix_coordinate_form_add_entries(B, 1, &k, &itmp, &dtmp);
	}
      }
     }
  } else {
    list = MALLOC(sizeof(int)*n);
    dist = MALLOC(sizeof(real)*n);
    /*
    Dijkstra_khops(khops, D, 60, dist, &nlist, list, &dmax);
    for (j = 0; j < nlist; j++){
      fprintf(stderr,"{%d,%d}=%f,",60,list[j],dist[list[j]]);
    }
    fprintf(stderr,"\n");
    Dijkstra_khops(khops, D, 94, dist, &nlist, list, &dmax);
    for (j = 0; j < nlist; j++){
      fprintf(stderr,"{%d,%d}=%f,",94,list[j],dist[list[j]]);
    }
    fprintf(stderr,"\n");
    exit(1);

    */

    for (k = 0; k < n; k++){
      SparseMatrix_level_sets_khops(khops, D, k, &nlevel, &levelset_ptr, &levelset, &mask, FALSE);
      assert(nlevel-1 <= khops);/* the first level is the root */
      flag = Dijkstra_masked(D, k, dist, &nlist, list, &dmax, mask);
      assert(!flag);
      for (i = 0; i < nlevel; i++) {
	for (j = levelset_ptr[i]; j < levelset_ptr[i+1]; j++){
	  assert(mask[levelset[j]] == i+1);
	  mask[levelset[j]] = -1;
	}
      }
      for (j = 0; j < nlist; j++){
	itmp = list[j]; dtmp = dist[itmp];
	if (k != itmp) B = SparseMatrix_coordinate_form_add_entries(B, 1, &k, &itmp, &dtmp);
      }
   }
  }

  C = SparseMatrix_from_coordinate_format(B);
  SparseMatrix_delete(B);

  free(levelset_ptr);
  free(levelset);
  free(mask);
  free(dist);

  if (D != D0) SparseMatrix_delete(D);
  free(list);
  /* I can not find a reliable way to make the matrix symmetric. Right now I use a mask array to
     limit consider of only nodes with in k hops, but even this is not symmetric. e.g.,
     . 10  10    10  10
     .A---B---C----D----E
     .           2 |    | 2
     .             G----F
     .               2
     If we set hops = 4, and from A, it can not see F (which is 5 hops), hence distance(A,E) =40
     but from E, it can see all nodes (all within 4 hops), so distance(E, A)=36.
     .
     may be there is a better way to ensure symmetric, but for now we just symmetrize it
  */
  D = SparseMatrix_symmetrize(C, FALSE);
  SparseMatrix_delete(C);
  return D;

}
