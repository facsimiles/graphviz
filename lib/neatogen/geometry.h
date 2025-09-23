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

#include <stddef.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define PRIVATE __attribute__((visibility("hidden")))
#else
#define PRIVATE /* nothing */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_POINTF_S
    typedef pointf Point;
#else
    typedef struct Point {
	double x, y;
    } Point;
#endif

    extern double xmin, xmax, ymin, ymax;	/* extreme x,y values of sites */
    extern double deltax;	// xmax - xmin

    extern size_t nsites; // Number of sites
    extern int sqrt_nsites;

PRIVATE void geominit(void);
PRIVATE double dist_2(Point, Point); ///< distance squared between two points
PRIVATE void subpt(Point * a, Point b, Point c);
PRIVATE void addpt(Point * a, Point b, Point c);
PRIVATE double area_2(Point a, Point b, Point c);
PRIVATE int leftOf(Point a, Point b, Point c);
PRIVATE int intersection(Point a, Point b, Point c, Point d, Point * p);

#undef PRIVATE

#ifdef __cplusplus
}
#endif
