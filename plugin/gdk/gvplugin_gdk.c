/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <gvc/gvplugin.h>

extern gvplugin_installed_t gvdevice_gdk_types[];
extern gvplugin_installed_t gvloadimage_gdk_types[];

static gvplugin_api_t apis[] = {
    {API_device, gvdevice_gdk_types},
    {API_loadimage, gvloadimage_gdk_types},
    {(api_t)0, 0},
};

#ifdef GVDLL
#define GVPLUGIN_GDK_API __declspec(dllexport)
#else
#define GVPLUGIN_GDK_API
#endif

GVPLUGIN_GDK_API gvplugin_library_t gvplugin_gdk_LTX_library = {"gdk", apis};
