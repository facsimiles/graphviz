/// @file
/// @ingroup common_utils
/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<time.h>

typedef clock_t mytime_t;
#define GET_TIME(S) S = clock()
#define DIFF_IN_SECS(S,T) ((S - T) / (double)CLOCKS_PER_SEC)

#include <common/types.h>
#include <common/utils.h>

static mytime_t T;

void start_timer(void)
{
    GET_TIME(T);
}

double elapsed_sec(void)
{
    mytime_t S;
    double rv;

    GET_TIME(S);
    rv = DIFF_IN_SECS(S, T);
    return rv;
}
