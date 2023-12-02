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

#include "smyrnadefs.h"

#define ATTR_NOTEBOOK_IDX 6

_BB void on_txtAttr_changed(GtkWidget *widget, void *user_data);
_BB void on_attrApplyBtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_attrAddBtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_attrSearchBtn_clicked(GtkWidget *widget, void *user_data);
_BB void on_attrRB0_clicked(GtkWidget *widget, void *user_data);
_BB void on_attrProg_toggled(GtkWidget *widget, void *user_data);

extern attr_list *load_attr_list(Agraph_t *g);
extern void showAttrsWidget(void);
