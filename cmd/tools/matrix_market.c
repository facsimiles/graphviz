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

#include "matrix_market.h"
#include "mmio.h"
#include <sparse/SparseMatrix.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/alloc.h>

SparseMatrix SparseMatrix_import_matrix_market(FILE *f) {
  MM_typecode matcode;
  double *val = NULL;
  int *vali = NULL, m, n;
  void *vp = NULL;
  SparseMatrix A = NULL;
  int c;

  if ((c = fgetc(f)) != '%') {
    ungetc(c, f);
    return NULL;
  }
  ungetc(c, f);
  if (mm_read_banner(f, &matcode) != 0) {
#ifdef DEBUG
    printf("Could not process Matrix Market banner.\n");
#endif
    return NULL;
  }

  /* find out size of sparse matrix .... */
  size_t nz;
  if (mm_read_mtx_crd_size(f, &m, &n, &nz) != 0) {
    return NULL;
  }
  /* reseve memory for matrices */

  int *I = gv_calloc(nz, sizeof(int));
  int *J = gv_calloc(nz, sizeof(int));

  const int type = matcode.type;
  switch (type) {
  case MATRIX_TYPE_REAL:
    val = gv_calloc(nz, sizeof(double));
    for (size_t i = 0; i < nz; i++) {
      int num = fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
      if (num != 3) {
        goto done;
      }
      I[i]--; /* adjust from 1-based to 0-based */
      J[i]--;
    }
    if (matcode.shape == MS_SYMMETRIC) {
      I = gv_recalloc(I, nz, 2 * nz, sizeof(int));
      J = gv_recalloc(J, nz, 2 * nz, sizeof(int));
      val = gv_recalloc(val, nz, 2 * nz, sizeof(double));
      const size_t nzold = nz;
      for (size_t i = 0; i < nzold; i++) {
        if (I[i] != J[i]) {
          I[nz] = J[i];
          J[nz] = I[i];
          val[nz++] = val[i];
        }
      }
    } else if (matcode.shape == MS_SKEW) {
      I = gv_recalloc(I, nz, 2 * nz, sizeof(int));
      J = gv_recalloc(J, nz, 2 * nz, sizeof(int));
      val = gv_recalloc(val, nz, 2 * nz, sizeof(double));
      const size_t nzold = nz;
      for (size_t i = 0; i < nzold; i++) {
        if (I[i] == J[i]) { // skew symm should have no diag
          goto done;
        }
        I[nz] = J[i];
        J[nz] = I[i];
        val[nz++] = -val[i];
      }
    } else if (matcode.shape == MS_HERMITIAN) {
      goto done;
    }
    vp = val;
    break;
  case MATRIX_TYPE_INTEGER:
    vali = gv_calloc(nz, sizeof(int));
    for (size_t i = 0; i < nz; i++) {
      int num = fscanf(f, "%d %d %d\n", &I[i], &J[i], &vali[i]);
      if (num != 3) {
        goto done;
      }
      I[i]--; /* adjust from 1-based to 0-based */
      J[i]--;
    }
    if (matcode.shape == MS_SYMMETRIC) {
      I = gv_recalloc(I, nz, 2 * nz, sizeof(int));
      J = gv_recalloc(J, nz, 2 * nz, sizeof(int));
      vali = gv_recalloc(vali, nz, 2 * nz, sizeof(int));
      const size_t nzold = nz;
      for (size_t i = 0; i < nzold; i++) {
        if (I[i] != J[i]) {
          I[nz] = J[i];
          J[nz] = I[i];
          vali[nz++] = vali[i];
        }
      }
    } else if (matcode.shape == MS_SKEW) {
      I = gv_recalloc(I, nz, 2 * nz, sizeof(int));
      J = gv_recalloc(J, nz, 2 * nz, sizeof(int));
      vali = gv_recalloc(vali, nz, 2 * nz, sizeof(int));
      const size_t nzold = nz;
      for (size_t i = 0; i < nzold; i++) {
        if (I[i] == J[i]) { // skew symm should have no diag
          goto done;
        }
        I[nz] = J[i];
        J[nz] = I[i];
        vali[nz++] = -vali[i];
      }
    } else if (matcode.shape == MS_HERMITIAN) {
      goto done;
    }
    vp = vali;
    break;
  case MATRIX_TYPE_PATTERN:
    for (size_t i = 0; i < nz; i++) {
      int num = fscanf(f, "%d %d\n", &I[i], &J[i]);
      if (num != 2) {
        goto done;
      }
      I[i]--; /* adjust from 1-based to 0-based */
      J[i]--;
    }
    if (matcode.shape == MS_SYMMETRIC || matcode.shape == MS_SKEW) {
      I = gv_recalloc(I, nz, 2 * nz, sizeof(int));
      J = gv_recalloc(J, nz, 2 * nz, sizeof(int));
      const size_t nzold = nz;
      for (size_t i = 0; i < nzold; i++) {
        if (I[i] != J[i]) {
          I[nz] = J[i];
          J[nz++] = I[i];
        }
      }
    } else if (matcode.shape == MS_HERMITIAN) {
      goto done;
    }
    break;
  default:
    goto done;
  }

  A = SparseMatrix_from_coordinate_arrays(nz, m, n, I, J, vp, type,
                                          sizeof(double));
done:
  free(I);
  free(J);
  free(val);
  free(vali);

  if (A != NULL && matcode.shape == MS_SYMMETRIC) {
    A->is_symmetric = true;
    A->is_pattern_symmetric = true;
  }

  return A;
}
