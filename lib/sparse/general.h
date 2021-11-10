/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
/* Applications that do not use the common library can define STANDALONE
 * to get definitions/definitions that are normally provided there.
 * In particular, note that Verbose is declared but undefined.
 */
#ifndef STANDALONE
#include "cgraph.h"
#include "globals.h"
#include "logic.h"
#include "arith.h"
#include <common/memory.h>
#endif  /* STANDALONE */

#define real double

#define set_flag(a, flag) ((a)=((a)|(flag)))
#define test_flag(a, flag) ((a)&(flag))
#define clear_flag(a, flag) ((a) &=(~(flag)))

#ifdef STANDALONE
#define CALLOC calloc
#define MALLOC malloc
#define REALLOC realloc

#define N_NEW(n,t)   (t*)calloc((n),sizeof(t))
#define NEW(t)       (t*)malloc(sizeof(t))
#define MAX(a,b) ((a)>(b)?(a):b)
#define MIN(a,b) ((a)<(b)?(a):b)

#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#define PI 3.14159

#define POINTS(inch) 72*(inch)

typedef unsigned int boolean;
extern unsigned char Verbose;

#else  /* STANDALONE */
#define CALLOC gcalloc
#define MALLOC gmalloc
#define REALLOC grealloc
#endif    /* STANDALONE */

#ifndef DEBUG
#ifndef NDEBUG
#define NDEBUG /* switch off assert*/
#endif
#endif

#ifdef DEBUG
extern double _statistics[10];
#endif


extern int irand(int n);
extern real drand(void);
extern int *random_permutation(int n);/* random permutation of 0 to n-1 */


real* vector_subtract_to(int n, real *x, real *y);/* y = x-y */

real vector_product(int n, real *x, real *y);

real* vector_saxpy(int n, real *x, real *y, real beta); /* y = x+beta*y */


real* vector_saxpy2(int n, real *x, real *y, real beta);/* x = x+beta*y */

/* take m elements v[p[i]]],i=1,...,m and oput in u. u will be assigned if *u = NULL */
void vector_float_take(int n, float *v, int m, int *p, float **u);

/* give the position of the smallest, second smallest etc in vector v.
   results in p. If *p == NULL, p is assigned.
*/
void vector_ordering(int n, real *v, int **p);
void vector_sort_int(int n, int *v);

void vector_print(char *s, int n, real *x);

#define MACHINEACC 1.0e-16
#define SQRT_MACHINEACC 1.0e-8

#define MINDIST 1.e-15

enum {UNMATCHED = -1};


real distance(real *x, int dim, int i, int j);
real distance_cropped(real *x, int dim, int i, int j);

real point_distance(real *p1, real *p2, int dim);

char *strip_dir(char *s);

void scale_to_box(real xmin, real ymin, real xmax, real ymax, int n, int dim, real *x);
