/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <gvc/gvplugin.h>

extern gvplugin_installed_t gvdevice_devil_types[];

static gvplugin_api_t apis[] = {
    {API_device, gvdevice_devil_types},
    {(api_t)0, 0},
};

#ifdef GVDLL
#define GVPLUGIN_DEVIL_API __declspec(dllexport)
#else
#define GVPLUGIN_DEVIL_API
#endif

GVPLUGIN_DEVIL_API gvplugin_library_t gvplugin_devil_LTX_library = {"devil",
                                                                    apis};
