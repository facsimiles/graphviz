/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <pango/pangocairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* gv_ps_fontname;
    char* gv_font;
} gv_font_map;

extern gv_font_map* get_font_mapping(PangoFontMap * pfm);

#ifdef __cplusplus
}
#endif
