/*************************************************************************
 * Copyright (c) 2014 AT&T Intellectual Property 
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

#if defined(_WIN32) && !defined(LAB_GAMUT_EXPORTS)
#define EXTERN __declspec(dllimport)
#else
#define EXTERN /* nothing */
#endif

/** lookup table for the visible spectrum of the CIELAB color space
 *
 * This table is entries of 4-tuples of the form (L*, a*, b* lower bound,
 * b* upper bound). A plain lookup table and/or the use of structs is avoided to
 * save memory during compilation. Without this, MSVC ~2019 exhausts memory in
 * CI.
 *
 * More information about CIELAB:
 *   https://en.wikipedia.org/wiki/CIELAB_color_space
 */
EXTERN extern const signed char lab_gamut_data[];
EXTERN extern int lab_gamut_data_size;

#undef EXTERN

#ifdef __cplusplus
}
#endif
