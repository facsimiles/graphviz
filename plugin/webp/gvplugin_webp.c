/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <gvc/gvplugin.h>

extern gvplugin_installed_t gvdevice_webp_types[];
extern gvplugin_installed_t gvloadimage_webp_types[];

static gvplugin_api_t apis[] = {
    {API_device, gvdevice_webp_types},
    {API_loadimage, gvloadimage_webp_types},
    {(api_t)0, 0},
};

#ifdef GVDLL
#define GVPLUGIN_WEBP_API __declspec(dllexport)
#else
#define GVPLUGIN_WEBP_API
#endif

GVPLUGIN_WEBP_API gvplugin_library_t gvplugin_webp_LTX_library = { "webp", apis };
