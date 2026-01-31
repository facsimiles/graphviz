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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sparse/SparseMatrix.h>
#include <util/prisize_t.h>
#include <util/startswith.h>
#include <util/strcasecmp.h>

#include "mmio.h"

int mm_read_banner(FILE * f, MM_typecode * matcode)
{
    char line[MM_MAX_LINE_LENGTH];
    char banner[MM_MAX_TOKEN_LENGTH] = {0};
    char mtx[MM_MAX_TOKEN_LENGTH] = {0};
    char crd[MM_MAX_TOKEN_LENGTH] = {0};
    char data_type[MM_MAX_TOKEN_LENGTH] = {0};
    char storage_scheme[MM_MAX_TOKEN_LENGTH] = {0};

    *matcode = (MM_typecode){0};

    if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
	return MM_PREMATURE_EOF;

    // note: 63 == MM_MAX_TOKEN_LENGTH - 1
    if (sscanf(line, "%63s %63s %63s %63s %63s", banner, mtx, crd, data_type,
	       storage_scheme) != 5)
	return MM_PREMATURE_EOF;

    /* check for banner */
    if (!startswith(banner, MatrixMarketBanner))
	return MM_NO_HEADER;

    // first field should be “matrix”
    if (strcasecmp(mtx, MM_MTX_STR) != 0)
	return MM_UNSUPPORTED_TYPE;

    // second field describes whether this is a sparse matrix (in coordinate
    // storage) or a dense array

    if (strcasecmp(crd, MM_SPARSE_STR) != 0)
	return MM_UNSUPPORTED_TYPE;

    /* third field */

    if (strcasecmp(data_type, MM_REAL_STR) == 0)
	matcode->type = MATRIX_TYPE_REAL;
    else if (strcasecmp(data_type, MM_COMPLEX_STR) == 0)
	matcode->type = MATRIX_TYPE_COMPLEX;
    else if (strcasecmp(data_type, MM_PATTERN_STR) == 0)
	matcode->type = MATRIX_TYPE_PATTERN;
    else if (strcasecmp(data_type, MM_INT_STR) == 0)
	matcode->type = MATRIX_TYPE_INTEGER;
    else
	return MM_UNSUPPORTED_TYPE;


    /* fourth field */

    if (strcasecmp(storage_scheme, MM_GENERAL_STR) == 0)
	matcode->shape = MS_GENERAL;
    else if (strcasecmp(storage_scheme, MM_SYMM_STR) == 0)
	matcode->shape = MS_SYMMETRIC;
    else if (strcasecmp(storage_scheme, MM_HERM_STR) == 0)
	matcode->shape = MS_HERMITIAN;
    else if (strcasecmp(storage_scheme, MM_SKEW_STR) == 0)
	matcode->shape = MS_SKEW;
    else
	return MM_UNSUPPORTED_TYPE;


    return 0;
}

int mm_read_mtx_crd_size(FILE * f, int *M, int *N, size_t *nz) {
    char line[MM_MAX_LINE_LENGTH];
    int num_items_read;

    /* set return null parameter values, in case we exit with errors */
    *M = *N = 0;
    *nz = 0;

    /* now continue scanning until you reach the end-of-comments */
    do {
	if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
	    return MM_PREMATURE_EOF;
    } while (line[0] == '%');

    /* line[] is either blank or has M,N, nz */
    if (sscanf(line, "%d %d %" PRISIZE_T, M, N, nz) == 3)
	return 0;

    else
	do {
	    num_items_read = fscanf(f, "%d %d %" PRISIZE_T, M, N, nz);
	    if (num_items_read == EOF)
		return MM_PREMATURE_EOF;
	}
	while (num_items_read != 3);

    return 0;
}
