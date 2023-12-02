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

#include "gui.h"
#include "toolboxcallbacks.h"
#include <gtk/gtk.h>

#ifdef _MSC_VER // this is needed under MSVC to get libglade see the callback
#define _BB __declspec(dllexport)
#else
#define _BB
#endif

_BB void save_as_graph_clicked(GtkWidget *widget, void *user_data);
_BB void remove_graph_clicked(GtkWidget *widget, void *user_data);
_BB void btn_dot_clicked(GtkWidget *widget, void *user_data);
_BB void btn_neato_clicked(GtkWidget *widget, void *user_data);
_BB void btn_twopi_clicked(GtkWidget *widget, void *user_data);
_BB void btn_circo_clicked(GtkWidget *widget, void *user_data);
_BB void btn_fdp_clicked(GtkWidget *widget, void *user_data);

_BB void graph_select_change(GtkWidget *widget, void *user_data);
_BB void mGraphPropertiesSlot(GtkWidget *widget, void *user_data);

/*console output widgets*/
_BB void on_clearconsolebtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_hideconsolebtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_consoledecbtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_consoleincbtn_clicked(GtkWidget *widget, void *user_data);
