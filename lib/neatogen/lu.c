/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * This code was (mostly) written by Ken Turkowski, who said:
 *
 * Oh, that. I wrote it in college the first time. It's open source - I think I
 * posted it after seeing so many people solve equations by inverting matrices
 * by computing minors naïvely.
 * -Ken
 *
 * The views represented here are mine and are not necessarily shared by
 * my employer.
   	Ken Turkowski			turk@apple.com
	Immersive Media Technologist 	http://www.worldserver.com/turk/
	Apple Computer, Inc.
	1 Infinite Loop, MS 302-3VR
	Cupertino, CA 95014
 */



/* This module solves linear equations in several variables (Ax = b) using
 * LU decomposition with partial pivoting and row equilibration.  Although
 * slightly more work than Gaussian elimination, it is faster for solving
 * several equations using the same coefficient matrix.  It is
 * particularly useful for matrix inversion, by sequentially solving the
 * equations with the columns of the unit matrix.
 *
 * lu_decompose() decomposes the coefficient matrix into the LU matrix,
 * and lu_solve() solves the series of matrix equations using the
 * previous LU decomposition.
 *
 *	Ken Turkowski (apple!turk)
 *	written 3/2/79, revised and enhanced 8/9/83.
 */

#include "config.h"

#include <assert.h>
#include <math.h>
#include <neatogen/neato.h>
#include <stdlib.h>
#include <util/alloc.h>
#include <util/gv_math.h>

/* lu_decompose() decomposes the coefficient matrix A into upper and lower
 * triangular matrices, the composite being the LU matrix.
 *
 * The arguments are:
 *
 *	lu - the state for the computation
 *	a - the (n x n) coefficient matrix
 *	n - the order of the matrix
 *
 *  1 is returned if the decomposition was successful,
 *  and 0 is returned if the coefficient matrix is singular.
 */

int lu_decompose(lu_t *lu, double **a, int n) {
    assert(lu != NULL);

    lu->lu = new_array(n, n, 0.0);
    lu->ps = gv_calloc(n, sizeof(int));
    double *const scales = gv_calloc(n, sizeof(double));

    for (int i = 0; i < n; i++) { // for each row
	/* Find the largest element in each row for row equilibration */
	double biggest = 0;
	for (int j = 0; j < n; j++)
	    biggest = fmax(biggest, fabs(lu->lu[i][j] = a[i][j]));
	if (biggest > 0.0)
	    scales[i] = 1.0 / biggest;
	else {
	    free(scales);
	    lu_free(lu);
	    return 0;		/* Zero row: singular matrix */
	}
	lu->ps[i] = i; // initialize pivot sequence
    }

    for (int k = 0, pivotindex = 0; k < n - 1; k++) { // for each column
	/* Find the largest element in each column to pivot around */
	double biggest = 0;
	for (int i = k; i < n; i++) {
	    const double tempf = fabs(lu->lu[lu->ps[i]][k]) * scales[lu->ps[i]];
	    if (biggest < tempf) {
		biggest = tempf;
		pivotindex = i;
	    }
	}
	if (biggest <= 0.0) {
	    free(scales);
	    lu_free(lu);
	    return 0;		/* Zero column: singular matrix */
	}
	if (pivotindex != k) {	/* Update pivot sequence */
	    SWAP(&lu->ps[k], &lu->ps[pivotindex]);
	}

	/* Pivot, eliminating an extra variable  each time */
	const double pivot = lu->lu[lu->ps[k]][k];
	for (int i = k + 1; i < n; i++) {
	    const double mult = lu->lu[lu->ps[i]][k] = lu->lu[lu->ps[i]][k] / pivot;
	    for (int j = k + 1; j < n; j++)
		lu->lu[lu->ps[i]][j] -= mult * lu->lu[lu->ps[k]][j];
	}
    }

    free(scales);
    if (lu->lu[lu->ps[n - 1]][n - 1] == 0.0) {
	lu_free(lu);
	return 0;		/* Singular matrix */
    }
    return 1;
}

/* lu_solve() solves the linear equation (Ax = b) after the matrix A has
 * been decomposed with lu_decompose() into the lower and upper triangular
 * matrices L and U.
 *
 * The arguments are:
 *
 *	lu - the state for the computation
 *	x - the solution vector
 *	b - the constant vector
 *	n - the order of the equation
*/

void lu_solve(const lu_t *lu, double *x, double *b, int n) {
    double dot;

    /* Vector reduction using U triangular matrix */
    for (int i = 0; i < n; i++) {
	dot = 0.0;
	for (int j = 0; j < i; j++)
	    dot += lu->lu[lu->ps[i]][j] * x[j];
	x[i] = b[lu->ps[i]] - dot;
    }

    /* Back substitution, in L triangular matrix */
    for (int i = n - 1; i >= 0; i--) {
	dot = 0.0;
	for (int j = i + 1; j < n; j++)
	    dot += lu->lu[lu->ps[i]][j] * x[j];
	x[i] = (x[i] - dot) / lu->lu[lu->ps[i]][i];
    }
}

void lu_free(lu_t *lu) {
  if (lu == NULL) {
    return;
  }
  if (lu->lu != NULL) {
    free_array(lu->lu);
  }
  free(lu->ps);
}
