/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "builddate.h"
#include "config.h"
// windows.h for win machines
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <windowsx.h>
#endif
#include "frmobjectui.h"
#include "glexpose.h"
#include "gltemplate.h"
#include "glutrender.h"
#include "gui.h"
#include "gvprpipe.h"
#include "menucallbacks.h"
#include "viewport.h"
#include <assert.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/gv_find_me.h>

static char *smyrnaDir; ///< path to directory containing smyrna data files
static char *smyrnaGlade;

/* smyrnaPath:
 * Construct pathname for smyrna data file.
 * Base file name is given as suffix.
 * The function resolves the directory containing the data files,
 * and constructs a complete pathname.
 * The returned string is malloced, so the application should free
 * it later.
 * Returns NULL on error.
 */
char *smyrnaPath(char *suffix) {
  static size_t baselen;
#ifdef _WIN32
  char *pathSep = "\\";
#else
  char *pathSep = "/";
#endif
  assert(smyrnaDir);

  if (baselen == 0) {
    baselen = strlen(smyrnaDir) + 2;
  }
  size_t len = baselen + strlen(suffix);
  char *buf = gv_calloc(len, sizeof(char));
  snprintf(buf, len, "%s%s%s", smyrnaDir, pathSep, suffix);
  return buf;
}

static char *useString = "Usage: smyrna [-v?] <file>\n\
  -f<WxH:bits@rate>         - full-screen mode\n\
  -e         - draw edges as splines if available\n\
  -v         - verbose\n\
  -?         - print usage\n";

static void usage(int v) {
  fputs(useString, stdout);
  graphviz_exit(v);
}

static char *Info[] = {
    "smyrna",        /* Program */
    PACKAGE_VERSION, /* Version */
    BUILDDATE        /* Build Date */
};

static char *parseArgs(int argc, char *argv[], ViewInfo *viewinfo) {
  int c;

  while ((c = getopt(argc, argv, ":eKf:txvV?")) != -1) {
    switch (c) {
    case 'e':
      viewinfo->drawSplines = 1;
      break;
    case 'v': // FIXME: deprecate and remove -v in future
      break;
    case 'f':
      viewinfo->guiMode = GUI_FULLSCREEN;
      viewinfo->optArg = optarg;
      break;

    case 'V':
      fprintf(stderr, "%s version %s (%s)\n", Info[0], Info[1], Info[2]);
      graphviz_exit(0);
      break;
    case '?':
      if (optopt == '\0' || optopt == '?')
        usage(0);
      else {
        fprintf(stderr, "smyrna: option -%c unrecognized\n", optopt);
        usage(1);
      }
      break;
    }
  }

  if (optind < argc)
    return argv[optind];
  else
    return NULL;
}

static void windowedMode(int argc, char *argv[]) {
  GdkGLConfig *glconfig;
  /*combo box to show loaded graphs */
  GtkComboBox *graphComboBox;

  gtk_set_locale();
  gtk_init(&argc, &argv);
  if (!(smyrnaGlade))
    smyrnaGlade = smyrnaPath("smyrna.glade");
  xml = glade_xml_new(smyrnaGlade, NULL, NULL);

  GtkWidget *gladewidget = glade_xml_get_widget(xml, "frmMain");
  gtk_widget_show(gladewidget);
  g_signal_connect(gladewidget, "destroy", G_CALLBACK(mQuitSlot), NULL);
  glade_xml_signal_autoconnect(xml);
  gtk_gl_init(0, 0);
  /* Configure OpenGL framebuffer. */
  glconfig = configure_gl();
  gladewidget = glade_xml_get_widget(xml, "hbox11");

  gtk_widget_hide(glade_xml_get_widget(xml, "vbox13"));
  gtk_window_set_deletable(
      (GtkWindow *)glade_xml_get_widget(xml, "dlgSettings"), 0);
  gtk_window_set_deletable((GtkWindow *)glade_xml_get_widget(xml, "frmTVNodes"),
                           0);
  create_window(glconfig, gladewidget);
  change_cursor(GDK_TOP_LEFT_ARROW);

#ifndef _WIN32
  glutInit(&argc, argv);
#endif

  gladewidget = glade_xml_get_widget(xml, "hbox13");
  graphComboBox = (GtkComboBox *)gtk_combo_box_new_text();
  gtk_box_pack_end((GtkBox *)gladewidget, (GtkWidget *)graphComboBox, 1, 1, 10);
  gtk_widget_show((GtkWidget *)graphComboBox);
  view->graphComboBox = graphComboBox;

  if (view->guiMode != GUI_FULLSCREEN)
    gtk_main();
}

/// find an absolute path to where Smyrna auxiliary files are stored
static char *find_share(void) {

#ifdef _WIN32
  const char PATH_SEPARATOR = '\\';
#else
  const char PATH_SEPARATOR = '/';
#endif

  // find the path to the `smyrna` binary
  char *smyrna_exe = gv_find_me();
  if (smyrna_exe == NULL) {
    fputs("failed to find path to self\n", stderr);
    graphviz_exit(EXIT_FAILURE);
  }

  // assume it is of the form …/bin/smyrna[.exe] and construct
  // …/share/graphviz/smyrna

  char *slash = strrchr(smyrna_exe, PATH_SEPARATOR);
  if (slash == NULL) {
    fprintf(stderr, "no path separator in path to self, %s\n", smyrna_exe);
    free(smyrna_exe);
    graphviz_exit(EXIT_FAILURE);
  }

  *slash = '\0';
  slash = strrchr(smyrna_exe, PATH_SEPARATOR);
  if (slash == NULL) {
    fprintf(stderr, "no path separator in directory containing self, %s\n",
            smyrna_exe);
    free(smyrna_exe);
    graphviz_exit(EXIT_FAILURE);
  }

  *slash = '\0';
  size_t share_len = strlen(smyrna_exe) + strlen("/share/graphviz/smyrna") + 1;
  char *share = gv_alloc(share_len);
  snprintf(share, share_len, "%s%cshare%cgraphviz%csmyrna", smyrna_exe,
           PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR);
  free(smyrna_exe);

  return share;
}

int main(int argc, char *argv[]) {
  smyrnaDir = getenv("SMYRNA_PATH");
  if (!smyrnaDir) {
    smyrnaDir = find_share();
  }

  char *package_locale_dir;
#ifdef G_OS_WIN32
  {
    char *package_prefix =
        g_win32_get_package_installation_directory(NULL, NULL);
    package_locale_dir =
        g_build_filename(package_prefix, "share", "locale", NULL);
    g_free(package_prefix);
  }
#else
  package_locale_dir = g_build_filename(smyrnaDir, "locale", NULL);
#endif /* # */
  view = gv_alloc(sizeof(ViewInfo));
  init_viewport(view);
  view->initFileName = parseArgs(argc, argv, view);
  if (view->initFileName)
    view->initFile = 1;

  if (view->guiMode == GUI_FULLSCREEN)
    cb_glutinit(800, 600, &argc, argv, view->optArg);
  else
    windowedMode(argc, argv);
  g_free(package_locale_dir);
  graphviz_exit(0);
}

/**
 * @dir .
 * @brief interactive graph viewer
 */
