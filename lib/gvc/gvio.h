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

#include "gvcjob.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GVDLL
#define GVIO_API __declspec(dllexport)
#else
#define GVIO_API
#endif

/*visual studio*/
#ifdef _WIN32
#ifndef GVC_EXPORTS
#undef GVIO_API
#define GVIO_API __declspec(dllimport)
#endif
#endif
/*end visual studio*/
#ifndef GVIO_API
#define GVIO_API extern
#endif

    GVIO_API size_t gvwrite (GVJ_t * job, const char *s, size_t len);
    GVIO_API int gvferror (FILE *stream);
    GVIO_API int gvputc(GVJ_t * job, int c);
    GVIO_API int gvputs(GVJ_t * job, const char *s);

    // `gvputs`, but XML-escape the input string
    GVIO_API int gvputs_xml(GVJ_t* job, const char *s);

    GVIO_API int gvflush (GVJ_t * job);
    GVIO_API void gvprintf(GVJ_t * job, const char *format, ...);
    GVIO_API void gvprintdouble(GVJ_t * job, double num);
    GVIO_API void gvprintpointf(GVJ_t * job, pointf p);
    GVIO_API void gvprintpointflist(GVJ_t * job, pointf *p, int n);

#undef GVIO_API

#ifdef __cplusplus
}
#endif
