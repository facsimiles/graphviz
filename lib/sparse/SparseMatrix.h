/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <sparse/general.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYMMETRY_EPSILON 0.0000001
enum {FORMAT_CSR, FORMAT_COORD};
enum {UNMASKED = -10, MASKED = 1};
enum {BIPARTITE_RECT = 0, BIPARTITE_PATTERN_UNSYM, BIPARTITE_UNSYM, BIPARTITE_ALWAYS};


struct SparseMatrix_struct {
  int m; /* row dimension */
  int n; /* column dimension */
  size_t nz; ///< the actual length used is nz, for CSR/CSC matrix this is the same as ia[n]
  size_t nzmax; ///< the current length of ja and a (if exists) allocated
  int type; /* whether it is real/complex matrix, or pattern only */
  int *ia; /* row pointer for CSR format, or row indices for coordinate format. 0-based */
  int *ja; /* column indices. 0-based */
  void *a; /* entry values. If NULL, pattern matrix */
  int format;/* whether it is CSR, CSC, COORD. By default it is in CSR format */
  bool is_pattern_symmetric :1;
  bool is_symmetric :1;
  bool is_undirected :1;
  size_t size;/* size of each entry. This allows for general matrix where each entry is, say, a matrix itself */
};

typedef struct SparseMatrix_struct* SparseMatrix;

enum {MATRIX_TYPE_REAL = 1<<0, MATRIX_TYPE_INTEGER = 1<<2, MATRIX_TYPE_PATTERN = 1<<3};

SparseMatrix SparseMatrix_new(int m, int n, size_t nz, int type, int format);

/* this version sum repeated entries */
SparseMatrix SparseMatrix_from_coordinate_format(SparseMatrix A);
SparseMatrix SparseMatrix_from_coordinate_format_not_compacted(SparseMatrix A);

SparseMatrix SparseMatrix_from_coordinate_arrays(size_t nz, int m, int n,
                                                 int *irn, int *jcn,
                                                 const void *val, int type,
                                                 size_t sz);

SparseMatrix SparseMatrix_from_coordinate_arrays_not_compacted(size_t nz, int m,
                                                               int n, int *irn,
                                                               int *jcn,
                                                               void *val,
                                                               int type,
                                                               size_t sz);

void SparseMatrix_export(FILE *f, SparseMatrix A);/* export into MM format except the header */

void SparseMatrix_delete(SparseMatrix A);

SparseMatrix SparseMatrix_add(SparseMatrix A, SparseMatrix B);
SparseMatrix SparseMatrix_multiply(SparseMatrix A, SparseMatrix B);
SparseMatrix SparseMatrix_multiply3(SparseMatrix A, SparseMatrix B, SparseMatrix C);

enum {SUM_REPEATED_NONE = 0, SUM_REPEATED_ALL, };
SparseMatrix SparseMatrix_sum_repeat_entries(SparseMatrix A);

SparseMatrix SparseMatrix_coordinate_form_add_entry_(SparseMatrix A, int irn,
                                                     int jcn, const void *val,
                                                     int type);

/// wrap `SparseMatrix_coordinate_form_add_entry_` for type safety
#ifndef __cplusplus
#define SparseMatrix_coordinate_form_add_entry(A, irn, jcn, val)               \
  SparseMatrix_coordinate_form_add_entry_((A), (irn), (jcn), (val),            \
    _Generic((val),                                                            \
      double *: MATRIX_TYPE_REAL,                                              \
      int *:MATRIX_TYPE_INTEGER,                                               \
      void *:MATRIX_TYPE_PATTERN                                               \
  ))
#endif
bool SparseMatrix_is_symmetric(SparseMatrix A, bool test_pattern_symmetry_only);
SparseMatrix SparseMatrix_transpose(SparseMatrix A);
SparseMatrix SparseMatrix_symmetrize(SparseMatrix A,
                                     bool pattern_symmetric_only);
void SparseMatrix_multiply_vector(SparseMatrix A, double *v, double **res);/* if v = NULL, v is assumed to be {1,1,...,1}*/
SparseMatrix SparseMatrix_remove_diagonal(SparseMatrix A);
SparseMatrix SparseMatrix_remove_upper(SparseMatrix A);/* remove diag and upper diag */
SparseMatrix SparseMatrix_divide_row_by_degree(SparseMatrix A);
SparseMatrix SparseMatrix_get_real_adjacency_matrix_symmetrized(SparseMatrix A);  /* symmetric, all entries to 1, diaginal removed */
void SparseMatrix_multiply_dense(SparseMatrix A, const double *v, double *res,
                                 int dim);
SparseMatrix SparseMatrix_apply_fun(SparseMatrix A, double (*fun)(double x));/* for real only! */
SparseMatrix SparseMatrix_copy(SparseMatrix A);
bool SparseMatrix_has_diagonal(SparseMatrix A);
SparseMatrix SparseMatrix_make_undirected(SparseMatrix A);/* make it strictly low diag only, and set flag to undirected */
int *SparseMatrix_weakly_connected_components(SparseMatrix A0, int *ncomp,
                                              int **comps);
void SparseMatrix_decompose_to_supervariables(SparseMatrix A, int *ncluster, int **cluster, int **clusterp);
SparseMatrix SparseMatrix_get_submatrix(SparseMatrix A, int nrow, int ncol, int *rindices, int *cindices);

SparseMatrix SparseMatrix_get_augmented(SparseMatrix A);

/* bipartite_options:
   BIPARTITE_RECT -- turn rectangular matrix into square), 
   BIPARTITE_PATTERN_UNSYM -- pattern unsummetric as bipartite
   BIPARTITE_UNSYM -- unsymmetric as square
   BIPARTITE_ALWAYS -- always as square
*/
SparseMatrix SparseMatrix_to_square_matrix(SparseMatrix A, int bipartite_options);

SparseMatrix SparseMatrix_sort(SparseMatrix A);

SparseMatrix SparseMatrix_set_entries_to_real_one(SparseMatrix A);

/// compute a distance matrix
///
/// @param D0 The graph. Entry values are unused.
/// @return An n×n matrix, (i,j)-th entry gives the distance of node i to j
SparseMatrix SparseMatrix_distance_matrix(SparseMatrix D0);

#ifdef __cplusplus
}

/// reimplement C `SparseMatrix_coordinate_form_add_entry` as an overloaded C++
/// function because C++ has no `_Generic`
static inline SparseMatrix
SparseMatrix_coordinate_form_add_entry(SparseMatrix A, int irn, int jcn,
                                       const double *val) {
  return SparseMatrix_coordinate_form_add_entry_(A, irn, jcn, val,
                                                 MATRIX_TYPE_REAL);
}
static inline SparseMatrix
SparseMatrix_coordinate_form_add_entry(SparseMatrix A, int irn, int jcn,
                                       const int *val) {
  return SparseMatrix_coordinate_form_add_entry_(A, irn, jcn, val,
                                                 MATRIX_TYPE_INTEGER);
}
static inline SparseMatrix
SparseMatrix_coordinate_form_add_entry(SparseMatrix A, int irn, int jcn,
                                       std::nullptr_t) {
  return SparseMatrix_coordinate_form_add_entry_(A, irn, jcn, nullptr,
                                                 MATRIX_TYPE_PATTERN);
}

#endif
