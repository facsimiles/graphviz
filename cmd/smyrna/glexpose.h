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

#include "viewport.h"

/// any time screen needs to be redrawn, this function is called by gltemplate
///
/// All drawings are initialized in this function.
///
/// @param v Global view variable defined in viewport.c
/// @return 0 if something goes wrong with GL, 1 otherwise
int glexpose_main(ViewInfo *vi);
