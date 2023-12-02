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

// file
_BB void mAttributesSlot(GtkWidget *widget, void *user_data);
_BB void mOpenSlot(GtkWidget *widget, void *user_data);
_BB void mSaveSlot(GtkWidget *widget, void *user_data);
_BB void mSaveAsSlot(GtkWidget *widget, void *user_data);
_BB void mCloseSlot(GtkWidget *widget, void *user_data);
_BB void mOptionsSlot(GtkWidget *widget, void *user_data);
_BB void mQuitSlot(GtkWidget *widget, void *user_data);
// edit
_BB void mCutSlot(GtkWidget *widget, void *user_data);
_BB void mCopySlot(GtkWidget *widget, void *user_data);
_BB void mPasteSlot(GtkWidget *widget, void *user_data);
_BB void mDeleteSlot(GtkWidget *widget, void *user_data);
_BB void mTopviewSettingsSlot(GtkWidget *widget, void *user_data);
_BB void mNodeFindSlot(GtkWidget *widget, void *user_data);

// view
_BB void mShowToolBoxSlot(GtkWidget *widget, void *user_data);
_BB void mShowHostSelectionSlot(GtkWidget *widget, void *user_data);
_BB void mMenuPan(GtkWidget *widget, void *user_data);
_BB void mMenuZoom(GtkWidget *widget, void *user_data);
_BB void mShowConsoleSlot(GtkWidget *widget, void *user_data);
_BB void mHideConsoleSlot(GtkWidget *widget, void *user_data);

// Graph
_BB void mNodeListSlot(GtkWidget *widget, void *user_data);
_BB void mNewNodeSlot(GtkWidget *widget, void *user_data);
_BB void mNewEdgeSlot(GtkWidget *widget, void *user_data);
_BB void mNewClusterSlot(GtkWidget *widget, void *user_data);
_BB void mGraphPropertiesSlot(GtkWidget *widget, void *user_data);
_BB void mClusterPropertiesSlot(GtkWidget *widget, void *user_data);
_BB void mNodePropertiesSlot(GtkWidget *widget, void *user_data);
_BB void mEdgePropertiesSlot(GtkWidget *widget, void *user_data);
_BB void mShowCodeSlot(GtkWidget *widget, void *user_data);

// select
_BB void mSelectAllSlot(GtkWidget *widget, void *user_data);
_BB void mUnselectAllSlot(GtkWidget *widget, void *user_data);
_BB void mSelectAllNodesSlot(GtkWidget *widget, void *user_data);
_BB void mSelectAllEdgesSlot(GtkWidget *widget, void *user_data);
_BB void mSelectAllClustersSlot(GtkWidget *widget, void *user_data);
_BB void mUnselectAllNodesSlot(GtkWidget *widget, void *user_data);
_BB void mUnselectAllEdgesSlot(GtkWidget *widget, void *user_data);
_BB void mUnselectAllClustersSlot(GtkWidget *widget, void *user_data);
_BB void mSingleSelectSlot(GtkWidget *widget, void *user_data);
_BB void mSelectAreaSlot(GtkWidget *widget, void *user_data);
_BB void mSelectAreaXSlot(GtkWidget *widget, void *user_data);

// help
_BB void mAbout(GtkWidget *widget, void *user_data);
_BB void mTestgvpr(GtkWidget *widget, void *user_data);
void change_cursor(GdkCursorType C);

/*others from settings dialog*/
_BB void on_gvprbuttonload_clicked(GtkWidget *widget, void *user_data);
_BB void on_gvprbuttonsave_clicked(GtkWidget *widget, void *user_data);
