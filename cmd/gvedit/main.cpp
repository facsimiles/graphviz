/**
 * @file
 * @brief gvedit - simple graph editor and viewer
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#ifdef _WIN32
#include "windows.h"
#endif
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <common/globals.h>
#include <gvc/gvc.h>
#include <stdio.h>
#include <util/debug.h>
#include <util/exit.h>

QTextStream errout(stderr, QIODevice::WriteOnly);

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(mdi);

  QStringList files;
  {
    // Scoped QCoreApplication for when X11 DISPLAY is not available
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("gvedit - simple graph editor and viewer"));
    parser.addPositionalArgument(
        QStringLiteral("files"),
        QCoreApplication::translate("main", "files to open."),
        QStringLiteral("[files...]"));

    const QCommandLineOption helpOption(
        {
            QStringLiteral("?"),
            QStringLiteral("h"),
            QStringLiteral("help"),
        },
        QCoreApplication::translate("main",
                                    "Displays help on commandline options."));
    parser.addOption(helpOption);

    const QCommandLineOption scaleInputBy72Option(
        {
            QStringLiteral("s"),
            QStringLiteral("scale-input-by-72"),
        },
        QCoreApplication::translate("main", "Scale input by 72"));
    parser.addOption(scaleInputBy72Option);

    const QCommandLineOption verboseOption(
        {
            QStringLiteral("v"),
            QStringLiteral("verbose"),
        },
        QCoreApplication::translate("main", "Verbose mode"));
    parser.addOption(verboseOption);

    if (!parser.parse(app.arguments())) {
      parser.showHelp(1);
    }

    if (parser.isSet(helpOption)) {
      parser.showHelp(0);
    }

    if (parser.isSet(scaleInputBy72Option)) {
      PSinputscale = POINTS_PER_INCH;
    }

    if (parser.isSet(verboseOption)) {
      Verbose = 1;
    }

    files = parser.positionalArguments();
  }

  QApplication app(argc, argv);

  CMainWindow mainWin(files);
  mainWin.show();
  const int ret = app.exec();
  graphviz_exit(ret);
}

/**
 * @dir .
 * @brief simple graph editor and viewer
 */
