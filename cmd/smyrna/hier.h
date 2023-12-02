/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <topfish/hierarchy.h>

    typedef struct {
	int num_foci;
	int *foci_nodes;	/* Nodes in real graph */
	double *x_foci;		/* Universal coordinates */
	double *y_foci;
    } focus_t;

    typedef struct {
  // All 5 must be set
	int width;		/* viewport width */
	int height;		/* viewport height */
	double distortion;	/* default of 1.0 */
    } reposition_t;

    void positionAllItems(Hierarchy * hp, focus_t * fs,
			  reposition_t * parms);
    Hierarchy *makeHier(int nnodes, int nedges, v_data *, double *,
			double *, hierparms_t *);

    focus_t *initFocus(int ncnt);
