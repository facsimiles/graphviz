/**
 * @file
 * @brief <a href=https://math.nist.gov/MatrixMarket/>Matrix Market</a> I/O API
 */

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
*   Matrix Market I/O library for ANSI C
*
*   See http://math.nist.gov/MatrixMarket for details.
*
*
*/

#pragma once

#include <stddef.h>
#include <stdio.h>

#define MM_MAX_LINE_LENGTH 100025
#define MatrixMarketBanner "%%MatrixMarket"
#define MM_MAX_TOKEN_LENGTH 64

typedef enum { MS_GENERAL, MS_SYMMETRIC, MS_HERMITIAN, MS_SKEW } matrix_shape_t;

int mm_read_banner(FILE *f, matrix_shape_t *shape);
int mm_read_mtx_crd_size(FILE * f, int *M, int *N, size_t *nz);

/********************* Matrix Market error codes ***************************/


#define MM_COULD_NOT_READ_FILE	11
#define MM_PREMATURE_EOF		12
#define MM_NOT_MTX				13
#define MM_NO_HEADER			14
#define MM_UNSUPPORTED_TYPE		15
#define MM_LINE_TOO_LONG		16

/******************** Matrix Market internal definitions ********************

   MM_matrix_typecode: 4-character sequence

				    ojbect 		sparse/   	data        storage 
						  		dense     	type        scheme

   string position:	 [0]        [1]			[2]         [3]

   Matrix typecode:  M(atrix)  C(oord)		R(eal)   	G(eneral)
						        A(array)	C(omplex)   H(ermitian)
											P(attern)   S(ymmetric)
								    		I(nteger)	K(kew)

 ***********************************************************************/

#define MM_MTX_STR		"matrix"
#define MM_COORDINATE_STR "coordinate"
#define MM_SPARSE_STR	"coordinate"
#define MM_REAL_STR		"real"
#define MM_GENERAL_STR  "general"
#define MM_SYMM_STR		"symmetric"
#define MM_HERM_STR		"hermitian"
#define MM_SKEW_STR		"skew-symmetric"
