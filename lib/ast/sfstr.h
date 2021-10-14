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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * macro interface for sfio write strings
 */

#include <sfio/sfio.h>

#define sfstropen()	sfnew((Sfio_t*)0,(char*)0,-1,-1,SF_WRITE|SF_STRING)
#define sfstrclose(f)	sfclose(f)

#define sfstruse(f)	(sfputc(f,0), (char*)((f)->next = (f)->data) )

#ifdef __cplusplus
}
#endif
