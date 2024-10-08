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

#include "gui.h"
#include <gtk/gtk.h>
#ifdef _MSC_VER
#define _BB __declspec(dllexport)
#else
#define _BB /**/
#endif

_BB void btnToolZoomOut_clicked(GtkWidget *widget, void *user_data);
_BB void btnToolZoomFit_clicked(GtkWidget *widget, void *user_data);
_BB void btnToolFit_clicked(GtkWidget *widget, void *user_data);
_BB void on_btnActivateGraph_clicked(GtkWidget *widget, void *user_data);
