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

#include <stddef.h>

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

    /* Support for freelists */

    typedef struct freelist {
	struct freenode *head;	/* List of free nodes */
	struct freeblock *blocklist;	/* List of malloced blocks */
	int nodesize;		/* Size of node */
    } Freelist;

INTERNAL void *getfree(Freelist *);
INTERNAL void freeinit(Freelist *, int);
INTERNAL void makefree(void *, Freelist *);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
