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

#include <stdbool.h>
#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/sparsegraph.h>

PRIVATE void scadd(double *, int, double, double *);
PRIVATE double norm(double *, int);

PRIVATE void orthog1(int n, double *vec);
PRIVATE void init_vec_orth1(int n, double *vec);
PRIVATE void right_mult_with_vector(vtx_data *, int, double *,
				       double *);
PRIVATE void right_mult_with_vector_f(float **, int, double *,
					 double *);
PRIVATE void vectors_subtraction(int, double *, double *, double *);
PRIVATE void vectors_addition(int, double *, double *, double *);
PRIVATE void vectors_scalar_mult(int, const double *, double, double *);
PRIVATE void copy_vector(int n, const double *restrict source,
                         double *restrict dest);
PRIVATE double vectors_inner_product(int n, const double *vector1,
					const double *vector2);
PRIVATE double max_abs(int n, double *vector);

    /* sparse matrix extensions: */

PRIVATE void right_mult_with_vector_transpose
	(double **, int, int, double *, double *);
PRIVATE void right_mult_with_vector_d(double **, int, int, double *,
					 double *);
PRIVATE void mult_dense_mat(double **, float **, int, int, int,
			       float ***C);
PRIVATE void mult_dense_mat_d(double **, float **, int, int, int,
				 double ***CC);
PRIVATE void mult_sparse_dense_mat_transpose(vtx_data *, double **, int,
						int, float ***);
PRIVATE bool power_iteration(double **, int, int, double **, double *);


/*****************************
** Single precision (float) **
** version                  **
*****************************/

PRIVATE void orthog1f(int n, float *vec);
PRIVATE void right_mult_with_vector_ff(float *, int, float *, float *);
PRIVATE void vectors_subtractionf(int, float *, float *, float *);
PRIVATE void vectors_additionf(int n, float *vector1, float *vector2,
				  float *result);
PRIVATE void vectors_mult_additionf(int n, float *vector1, float alpha,
				       float *vector2);
PRIVATE void copy_vectorf(int n, float *source, float *dest);
PRIVATE double vectors_inner_productf(int n, float *vector1,
					 float *vector2);
PRIVATE void set_vector_val(int n, double val, double *result);
PRIVATE void set_vector_valf(int n, float val, float * result);
PRIVATE double max_absf(int n, float *vector);
PRIVATE void square_vec(int n, float *vec);
PRIVATE void invert_vec(int n, float *vec);
PRIVATE void sqrt_vecf(int n, float *source, float *target);
PRIVATE void invert_sqrt_vec(int n, float *vec);

#ifdef __cplusplus
}
#endif
