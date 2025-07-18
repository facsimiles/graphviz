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

#ifdef __cplusplus
extern "C" {
#endif

#include <neatogen/sparsegraph.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

INTERNAL void scadd(double *, int, double, double *);
INTERNAL double norm(double *, int);

INTERNAL void orthog1(int n, double *vec);
INTERNAL void init_vec_orth1(int n, double *vec);
INTERNAL void right_mult_with_vector(vtx_data *, int, double *,
				       double *);
INTERNAL void right_mult_with_vector_f(float **, int, double *,
					 double *);
INTERNAL void vectors_subtraction(int, double *, double *, double *);
INTERNAL void vectors_addition(int, double *, double *, double *);
INTERNAL void vectors_scalar_mult(int, const double *, double, double *);
INTERNAL void copy_vector(int n, const double *source, double *dest);
INTERNAL double vectors_inner_product(int n, const double *vector1,
					const double *vector2);
INTERNAL double max_abs(int n, double *vector);

    /* sparse matrix extensions: */

INTERNAL void right_mult_with_vector_transpose
	(double **, int, int, double *, double *);
INTERNAL void right_mult_with_vector_d(double **, int, int, double *,
					 double *);
INTERNAL void mult_dense_mat(double **, float **, int, int, int,
			       float ***C);
INTERNAL void mult_dense_mat_d(double **, float **, int, int, int,
				 double ***CC);
INTERNAL void mult_sparse_dense_mat_transpose(vtx_data *, double **, int,
						int, float ***);
INTERNAL bool power_iteration(double **, int, int, double **, double *);


/*****************************
** Single precision (float) **
** version                  **
*****************************/

INTERNAL void orthog1f(int n, float *vec);
INTERNAL void right_mult_with_vector_ff(float *, int, float *, float *);
INTERNAL void vectors_subtractionf(int, float *, float *, float *);
INTERNAL void vectors_additionf(int n, float *vector1, float *vector2,
				  float *result);
INTERNAL void vectors_mult_additionf(int n, float *vector1, float alpha,
				       float *vector2);
INTERNAL void copy_vectorf(int n, float *source, float *dest);
INTERNAL double vectors_inner_productf(int n, float *vector1,
					 float *vector2);
INTERNAL void set_vector_val(int n, double val, double *result);
INTERNAL void set_vector_valf(int n, float val, float * result);
INTERNAL double max_absf(int n, float *vector);
INTERNAL void square_vec(int n, float *vec);
INTERNAL void invert_vec(int n, float *vec);
INTERNAL void sqrt_vecf(int n, float *source, float *target);
INTERNAL void invert_sqrt_vec(int n, float *vec);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
