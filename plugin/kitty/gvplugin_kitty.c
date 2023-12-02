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

extern gvplugin_installed_t gvdevice_types_kitty[];

static gvplugin_api_t apis[] = {
    {API_device, gvdevice_types_kitty},
    {(api_t)0, 0},
};

#ifdef GVDLL
#define GVPLUGIN_KITTY_API __declspec(dllexport)
#else
#define GVPLUGIN_KITTY_API
#endif

GVPLUGIN_KITTY_API gvplugin_library_t gvplugin_kitty_LTX_library = {"kitty",
                                                                    apis};
