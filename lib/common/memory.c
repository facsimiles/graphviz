/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <cgraph/exit.h>
#include <stdio.h>
#include <stdlib.h>
#include <common/memory.h>

void *gcalloc(size_t nmemb, size_t size)
{
    char *rv = calloc(nmemb, size);
    if (nmemb > 0 && size > 0 && rv == NULL) {
	fprintf(stderr, "out of memory\n");
	graphviz_exit(EXIT_FAILURE);
    }
    return rv;
}

void *gmalloc(size_t nbytes)
{
    char *rv;
    if (nbytes == 0)
	return NULL;
    rv = malloc(nbytes);
    if (rv == NULL) {
	fprintf(stderr, "out of memory\n");
	graphviz_exit(EXIT_FAILURE);
    }
    return rv;
}

void *grealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (p == NULL && size) {
	fprintf(stderr, "out of memory\n");
	graphviz_exit(EXIT_FAILURE);
    }
    return p;
}
