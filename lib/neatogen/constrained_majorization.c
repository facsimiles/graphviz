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

#include <neatogen/digcola.h>
#ifdef DIGCOLA
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <neatogen/stress.h>
#include <neatogen/dijkstra.h>
#include <neatogen/bfs.h>
#include <neatogen/matrix_ops.h>
#include <neatogen/kkutils.h>
#include <neatogen/conjgrad.h>
#include <neatogen/quad_prog_solver.h>
#include <neatogen/matrix_ops.h>
#include <util/alloc.h>
#include <util/debug.h>

#define localConstrMajorIterations 15
#define levels_sep_tol 1e-1

int stress_majorization_with_hierarchy(vtx_data * graph,	/* Input graph in sparse representation  */
				       int n,	/* Number of nodes */
				       double **d_coords,	/* Coordinates of nodes (output layout)  */
				       node_t ** nodes,	/* Original nodes */
				       int dim,	/* Dimensionality of layout */
				       int opts,	/* options */
				       int model,	/* difference model */
				       int maxi,	/* max iterations */
				       double levels_gap)
{
    int iterations = 0;		/* Output: number of iteration of the process */

	/*************************************************
	** Computation of full, dense, unrestricted k-D ** 
	** stress minimization by majorization          ** 
	** This function imposes HIERARCHY CONSTRAINTS  **
	*************************************************/

    bool directionalityExist = false;
    float *lap1 = NULL;
    float *dist_accumulator = NULL;
    float *tmp_coords = NULL;
    float **b = NULL;
    double *degrees = NULL;
    float *lap2 = NULL;
    float *f_storage = NULL;
    float **coords = NULL;

    const double conj_tol = tolerance_cg; // tolerance of Conjugate Gradient
    CMajEnv *cMajEnv = NULL;
    const bool smart_ini = !!(opts & opt_smart_init);
    float *Dij = NULL;
    /* to compensate noises, we never consider gaps smaller than 'abs_tol' */
    const double abs_tol = 1e-2;
    /* Additionally, we never consider gaps smaller than 'abs_tol'*<avg_gap> */
    const double relative_tol = levels_sep_tol;
    int *ordering = NULL, *levels = NULL;
    bool converged;
    int num_levels;

    if (graph[0].edists != NULL) {
	for (int i = 0; i < n; i++) {
	    for (size_t j = 1; j < graph[i].nedges; j++) {
		directionalityExist |= graph[i].edists[j] != 0;
	    }
	}
    }
    if (!directionalityExist) {
	return stress_majorization_kD_mkernel(graph, n,
					      d_coords, nodes, dim, opts,
					      model, maxi);
    }

	/******************************************************************
	** First, partition nodes into layers: These are our constraints **
	******************************************************************/

    if (smart_ini) {
	if (dim > 2) {
	    /* the dim==2 case is handled below                      */
	    if (stress_majorization_kD_mkernel(graph, n,
					   d_coords + 1, nodes, dim - 1,
					   opts, model, 15) < 0)
		return -1;
	    /* now copy the y-axis into the (dim-1)-axis */
	    for (int i = 0; i < n; i++) {
		d_coords[dim - 1][i] = d_coords[1][i];
	    }
	}

	double *const x = d_coords[0];
	double *const y = d_coords[1];
	if (compute_y_coords(graph, n, y, n)) {
	    iterations = -1;
	    goto finish;
	}
	if (compute_hierarchy(graph, n, abs_tol, relative_tol, y, &ordering,
			  &levels, &num_levels)) {
	    iterations = -1;
	    goto finish;
	}
	if (num_levels < 1) {
	    /* no hierarchy found, use faster algorithm */
	    free(levels);
	    return stress_majorization_kD_mkernel(graph, n,
						  d_coords, nodes, dim,
						  opts, model, maxi);
	}

	if (levels_gap > 0) {
	    /* ensure that levels are separated in the initial layout */
	    double displacement = 0;
	    for (int i = 0; i < num_levels; i++) {
		displacement +=
		    MAX(0.0,
			levels_gap - (y[ordering[levels[i]]] +
				      displacement -
				      y[ordering[levels[i] - 1]]));
		const int stop = i < num_levels - 1 ? levels[i + 1] : n;
		for (int j = levels[i]; j < stop; j++) {
		    y[ordering[j]] += displacement;
		}
	    }
	}
	if (dim == 2) {
	    if (IMDS_given_dim(graph, n, y, x, Epsilon)) {
		iterations = -1;
		goto finish;
	    }
	}
    } else {
	initLayout(n, dim, d_coords, nodes);
	if (compute_hierarchy(graph, n, abs_tol, relative_tol, NULL, &ordering,
			  &levels, &num_levels)) {
	    iterations = -1;
	    goto finish;
	}
    }
    if (n == 1) {
	free(levels);
	return 0;
    }

	/****************************************************
	** Compute the all-pairs-shortest-distances matrix **
	****************************************************/

    if (maxi == 0) {
	free(levels);
	return iterations;
    }

    if (Verbose)
	start_timer();

    if (model == MODEL_SUBSET) {
	/* weight graph to separate high-degree nodes */
	/* and perform slower Dijkstra-based computation */
	if (Verbose)
	    fprintf(stderr, "Calculating subset model");
	Dij = compute_apsp_artificial_weights_packed(graph, n);
    } else if (model == MODEL_CIRCUIT) {
	Dij = circuitModel(graph, n);
	if (!Dij) {
	    agwarningf(
		  "graph is disconnected. Hence, the circuit model\n");
	    agerr(AGPREV,
		  "is undefined. Reverting to the shortest path model.\n");
	}
    } else if (model == MODEL_MDS) {
	if (Verbose)
	    fprintf(stderr, "Calculating MDS model");
	Dij = mdsModel(graph, n);
    }
    if (!Dij) {
	if (Verbose)
	    fprintf(stderr, "Calculating shortest paths");
	Dij = compute_apsp_packed(graph, n);
    }
    if (Verbose) {
	fprintf(stderr, ": %.2f sec\n", elapsed_sec());
	fprintf(stderr, "Setting initial positions");
	start_timer();
    }

    const int length = n + n * (n - 1) / 2;

    if (!smart_ini) {
	/* for numerical stability, scale down layout            */
	/* No Jiggling, might conflict with constraints                  */
	double max = 1;
	for (int i = 0; i < dim; i++) {
	    for (int j = 0; j < n; j++) {
		max = fmax(max, fabs(d_coords[i][j]));
	    }
	}
	for (int i = 0; i < dim; i++) {
	    for (int j = 0; j < n; j++) {
		d_coords[i][j] *= 10 / max;
	    }
	}
    }

    if (levels_gap > 0) {
	const double sum1 = n * (n - 1) / 2;
	double sum2 = 0;
	for (int count = 0, i = 0; i < n - 1; i++) {
	    count++;		// skip self distance
	    for (int j = i + 1; j < n; j++, count++) {
		sum2 += distance_kD(d_coords, dim, i, j) / Dij[count];
	    }
	}
	const float scale_ratio = (float)(sum2 / sum1);
	for (int i = 0; i < length; i++) {
	    Dij[i] *= scale_ratio;
	}
    }

	/**************************
	** Layout initialization **
	**************************/

    for (int i = 0; i < dim; i++) {
	orthog1(n, d_coords[i]);
    }

    /* for the y-coords, don't center them, but translate them so y[0]=0 */
    const double y_0 = d_coords[1][0];
    for (int i = 0; i < n; i++) {
	d_coords[1][i] -= y_0;
    }

    coords = gv_calloc(dim, sizeof(float *));
    f_storage = gv_calloc(dim * n, sizeof(float));
    for (int i = 0; i < dim; i++) {
	coords[i] = f_storage + i * n;
	for (int j = 0; j < n; j++) {
	    coords[i][j] = (float)d_coords[i][j];
	}
    }

    /* compute constant term in stress sum
     * which is \sum_{i<j} w_{ij}d_{ij}^2
     */
    const double constant_term = n * (n - 1) / 2;

    if (Verbose)
	fprintf(stderr, ": %.2f sec", elapsed_sec());

	/**************************
	** Laplacian computation **
	**************************/

    lap2 = Dij;
    const int lap_length = n + n * (n - 1) / 2;
    square_vec(lap_length, lap2);
    /* compute off-diagonal entries */
    invert_vec(lap_length, lap2);

    /* compute diagonal entries */
    degrees = gv_calloc(n, sizeof(double));
    set_vector_val(n, 0, degrees);
    for (int i = 0, count = 0; i < n - 1; i++) {
	double degree = 0;
	count++;		// skip main diag entry
	for (int j = 1; j < n - i; j++, count++) {
	    const float val = lap2[count];
	    degree += val;
	    degrees[i + j] -= val;
	}
	degrees[i] -= degree;
    }
    for (int step = n, count = 0, i = 0; i < n; i++, count += step, step--) {
	lap2[count] = (float) degrees[i];
    }

	/*************************
	** Layout optimization  **
	*************************/

    b = gv_calloc(dim, sizeof(float *));
    b[0] = gv_calloc(dim * n, sizeof(float));
    for (int k = 1; k < dim; k++) {
	b[k] = b[0] + k * n;
    }

    tmp_coords = gv_calloc(n, sizeof(float));
    dist_accumulator = gv_calloc(n, sizeof(float));
    lap1 = gv_calloc(lap_length, sizeof(float));

    double old_stress = DBL_MAX; // at least one iteration

    cMajEnv =
	initConstrainedMajorization(lap2, n, ordering, levels, num_levels);

    for (converged = false, iterations = 0;
	 iterations < maxi && !converged; iterations++) {

	/* First, construct Laplacian of 1/(d_ij*|p_i-p_j|)  */
	set_vector_val(n, 0, degrees);
	sqrt_vecf(lap_length, lap2, lap1);
	for (int count = 0, i = 0; i < n - 1; i++) {
	    const int len = n - i - 1;
	    /* init 'dist_accumulator' with zeros */
	    set_vector_valf(n, 0, dist_accumulator);

	    /* put into 'dist_accumulator' all squared distances 
	     * between 'i' and 'i'+1,...,'n'-1
	     */
	    for (int k = 0; k < dim; k++) {
		set_vector_valf(len, coords[k][i], tmp_coords);
		vectors_mult_additionf(len, tmp_coords, -1, coords[k] + i + 1);
		square_vec(len, tmp_coords);
		vectors_additionf(len, tmp_coords, dist_accumulator, dist_accumulator);
	    }

	    /* convert to 1/d_{ij} */
	    invert_sqrt_vec(len, dist_accumulator);
	    /* detect overflows */
	    for (int j = 0; j < len; j++) {
		if (dist_accumulator[j] >= FLT_MAX || dist_accumulator[j] < 0) {
		    dist_accumulator[j] = 0;
		}
	    }

	    count++;		/* save place for the main diagonal entry */
	    double degree = 0;
	    for (int j = 0; j < len; j++, count++) {
		const float val = lap1[count] *= dist_accumulator[j];
		degree += val;
		degrees[i + j + 1] -= val;
	    }
	    degrees[i] -= degree;
	}
	for (int step = n, count = 0, i = 0; i < n; i++, count += step, step--) {
	    lap1[count] = (float) degrees[i];
	}

	/* Now compute b[] (L^(X(t))*X(t)) */
	for (int k = 0; k < dim; k++) {
	    /* b[k] := lap1*coords[k] */
	    right_mult_with_vector_ff(lap1, n, coords[k], b[k]);
	}

	/* compute new stress
	 * remember that the Laplacians are negated, so we subtract 
	 * instead of add and vice versa
	 */
	double new_stress = 0;
	for (int k = 0; k < dim; k++) {
	    new_stress += vectors_inner_productf(n, coords[k], b[k]);
	}
	new_stress *= 2;
	new_stress += constant_term;	// only after mult by 2              
	for (int k = 0; k < dim; k++) {
	    right_mult_with_vector_ff(lap2, n, coords[k], tmp_coords);
	    new_stress -= vectors_inner_productf(n, coords[k], tmp_coords);
	}

	/* check for convergence */
	converged =
	    fabs(new_stress - old_stress) / fabs(old_stress + 1e-10) <
	    Epsilon;
	converged |= iterations > 1 && new_stress > old_stress;
	/* in first iteration we allowed stress increase, which 
	 * might result ny imposing constraints
	 */
	old_stress = new_stress;

	for (int k = 0; k < dim; k++) {
	    /* now we find the optimizer of trace(X'LX)+X'B by solving 'dim' 
	     * system of equations, thereby obtaining the new coordinates.
	     * If we use the constraints (given by the var's: 'ordering', 
	     * 'levels' and 'num_levels'), we cannot optimize 
	     * trace(X'LX)+X'B by simply solving equations, but we have
	     * to use a quadratic programming solver
	     * note: 'lap2' is a packed symmetric matrix, that is its 
	     * upper-triangular part is arranged in a vector row-wise
	     * also note: 'lap2' is really the negated laplacian (the 
	     * laplacian is -'lap2')
	     */

	    if (k == 1) {
		/* use quad solver in the y-dimension */
		constrained_majorization_new_with_gaps(cMajEnv, b[k],
						       coords, k,
						       localConstrMajorIterations,
						       levels_gap);

	    } else {
		/* use conjugate gradient for all dimensions except y */
		if (conjugate_gradient_mkernel(lap2, coords[k], b[k], n,
					   conj_tol, n)) {
		    iterations = -1;
		    goto finish;
		}
	    }
	}
    }

    for (int i = 0; i < dim; i++) {
	for (int j = 0; j < n; j++) {
	    d_coords[i][j] = coords[i][j];
	}
    }

    free(tmp_coords);
    free(dist_accumulator);
    free(degrees);
    free(lap2);

    free(lap1);

finish:
    if (cMajEnv != NULL) {
	deleteCMajEnv(cMajEnv);
    }
    if (b) {
	free(b[0]);
	free(b);
    }
    free(f_storage);
    free(coords);
    free(ordering);

    free(levels);

    return iterations;
}
#endif				/* DIGCOLA */
