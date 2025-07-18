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

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

#ifdef DIGCOLA

typedef struct {
	float **A;
	int n;
	float *fArray1;
	float *fArray2;
	float *fArray3;
	float *fArray4;
	int *ordering;
	int *levels;
	int num_levels;
}CMajEnv;

INTERNAL CMajEnv* initConstrainedMajorization(float *, int, int*, int*, int);

INTERNAL void constrained_majorization_new_with_gaps(CMajEnv*, float*, float**, 
                                            int, int, float);
INTERNAL void deleteCMajEnv(CMajEnv *e);

INTERNAL float** unpackMatrix(float * packedMat, int n);

#endif 

#undef INTERNAL

#ifdef __cplusplus
}
#endif
