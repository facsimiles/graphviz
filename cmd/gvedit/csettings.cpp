/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/
#ifdef _WIN32
#include "windows.h"
#endif
#include "csettings.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "string.h"
#include <QTemporaryFile>
#include <QtWidgets>
#include <cgraph/rdr.h>
#include <cstdlib>
#include <qfile.h>
#include <string>
#include <util/gv_find_me.h>

extern int errorPipe(char *errMsg);

#define WIDGET(t, f) (findChild<t *>(QStringLiteral(#f)))

/// wrapper around `gv_find_me` to convert to a C++ type
static std::string find_me() {
  char *const me = gv_find_me();
  const std::string me_s = me == NULL ? "" : me;
  free(me);
  return me_s;
}

/// find an absolute path to where Gvedit auxiliary files are stored
static std::string find_share(void) {

#ifdef _WIN32
  const char PATH_SEPARATOR = '\\';
#else
  const char PATH_SEPARATOR = '/';
#endif

  // find the path to the `gvedit` binary
  std::string gvedit_exe = find_me();
  if (gvedit_exe == "")
    return "";

  // assume it is of the form …/bin/gvedit[.exe] and construct
  // …/share/graphviz/gvedit

  size_t slash = gvedit_exe.rfind(PATH_SEPARATOR);
  if (slash == std::string::npos) {
    errout << "no path separator in path to self, " << gvedit_exe.c_str()
           << '\n';
    return "";
  }

  std::string bin = gvedit_exe.substr(0, slash);
  slash = bin.rfind(PATH_SEPARATOR);
  if (slash == std::string::npos) {
    errout << "no path separator in directory containing self, " << bin.c_str()
           << '\n';
    return "";
  }

  std::string install_prefix = bin.substr(0, slash);

  return install_prefix + PATH_SEPARATOR + "share" + PATH_SEPARATOR +
         "graphviz" + PATH_SEPARATOR + "gvedit";
}

bool loadAttrs(const QString &fileName, QComboBox *cbNameG, QComboBox *cbNameN,
               QComboBox *cbNameE) {
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd()) {
      line = stream.readLine(); // line of text excluding '\n'
      if (line.left(1) == QLatin1String(":")) {
        QString attrName;
        QStringList sl = line.split(u':');
        for (int id = 0; id < sl.count(); id++) {
          if (id == 1)
            attrName = sl[id];
          if (id == 2) {
            if (sl[id].contains(u'G'))
              cbNameG->addItem(attrName);
            if (sl[id].contains(u'N'))
              cbNameN->addItem(attrName);
            if (sl[id].contains(u'E'))
              cbNameE->addItem(attrName);
          }
        }
      }
    }
  } else {
    errout << "Could not open attribute name file \"" << fileName
           << "\" for reading\n";
    errout.flush();
    return true;
  }

  return false;
}

QString stripFileExtension(const QString &fileName) {
  // `lastIndexOf` returns -1 if not found and `left` takes a negative number to
  // mean “the entire string”, so this is a no-op if the filename has no
  // extension
  return fileName.left(fileName.lastIndexOf(u'.', fileName.size() - 1));
}

CFrmSettings::CFrmSettings() {
  this->gvc = gvContext();
  Ui_Dialog tempDia;
  tempDia.setupUi(this);
  graph = nullptr;
  activeWindow = nullptr;
  QString pathname;
  char *s = nullptr;
#ifndef _WIN32
  s = getenv("GVEDIT_PATH");
#endif
  if (s)
    pathname = QString::fromUtf8(s);
  else
    pathname = QString::fromStdString(find_share());

  connect(WIDGET(QPushButton, pbAdd), &QPushButton::clicked, this,
          &CFrmSettings::addSlot);
  connect(WIDGET(QPushButton, pbNew), &QPushButton::clicked, this,
          &CFrmSettings::newSlot);
  connect(WIDGET(QPushButton, pbOpen), &QPushButton::clicked, this,
          &CFrmSettings::openSlot);
  connect(WIDGET(QPushButton, pbSave), &QPushButton::clicked, this,
          &CFrmSettings::saveSlot);
  connect(WIDGET(QPushButton, btnOK), &QPushButton::clicked, this,
          &CFrmSettings::okSlot);
  connect(WIDGET(QPushButton, btnCancel), &QPushButton::clicked, this,
          &CFrmSettings::cancelSlot);
  connect(WIDGET(QPushButton, pbOut), &QPushButton::clicked, this,
          &CFrmSettings::outputSlot);
  connect(WIDGET(QPushButton, pbHelp), &QPushButton::clicked, this,
          &CFrmSettings::helpSlot);

  connect(WIDGET(QComboBox, cbScope),
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &CFrmSettings::scopeChangedSlot);
  scopeChangedSlot(0);

  if (!pathname.isEmpty()) {
    loadAttrs(pathname + QLatin1String("/attrs.txt"),
              WIDGET(QComboBox, cbNameG), WIDGET(QComboBox, cbNameN),
              WIDGET(QComboBox, cbNameE));
  }
  setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));
}

void CFrmSettings::outputSlot() {
  QString _filter = QStringLiteral("Output File(*.%1)")
                        .arg(WIDGET(QComboBox, cbExtension)->currentText());
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Graph As.."),
                                                  QStringLiteral("/"), _filter);
  if (!fileName.isEmpty())
    WIDGET(QLineEdit, leOutput)->setText(fileName);
}

void CFrmSettings::scopeChangedSlot(int id) {
  WIDGET(QComboBox, cbNameG)->setVisible(id == 0);
  WIDGET(QComboBox, cbNameN)->setVisible(id == 1);
  WIDGET(QComboBox, cbNameE)->setVisible(id == 2);
}

void CFrmSettings::addSlot() {
  QString _scope = WIDGET(QComboBox, cbScope)->currentText();
  QString _name;
  switch (WIDGET(QComboBox, cbScope)->currentIndex()) {
  case 0:
    _name = WIDGET(QComboBox, cbNameG)->currentText();
    break;
  case 1:
    _name = WIDGET(QComboBox, cbNameN)->currentText();
    break;
  case 2:
    _name = WIDGET(QComboBox, cbNameE)->currentText();
    break;
  }
  QString _value = WIDGET(QLineEdit, leValue)->text();

  if (_value.trimmed().isEmpty())
    QMessageBox::warning(this, tr("GvEdit"),
                         tr("Please enter a value for selected attribute!"),
                         QMessageBox::Ok, QMessageBox::Ok);
  else {
    QString str = _scope + QLatin1Char(u'[') + _name + QLatin1String("=\"");
    if (WIDGET(QTextEdit, teAttributes)->toPlainText().contains(str)) {
      QMessageBox::warning(this, tr("GvEdit"),
                           tr("Attribute is already defined!"), QMessageBox::Ok,
                           QMessageBox::Ok);
      return;
    }
    str = str + _value + QLatin1String("\"]");
    WIDGET(QTextEdit, teAttributes)
        ->setPlainText(WIDGET(QTextEdit, teAttributes)->toPlainText() + str +
                       QLatin1Char('\n'));
  }
}

void CFrmSettings::helpSlot() {
  QDesktopServices::openUrl(
      QUrl(QStringLiteral("http://www.graphviz.org/doc/info/attrs.html")));
}

void CFrmSettings::cancelSlot() { this->reject(); }

void CFrmSettings::okSlot() {
  saveContent();
  this->done(drawGraph());
}

void CFrmSettings::newSlot() {
  WIDGET(QTextEdit, teAttributes)->setPlainText(tr(""));
}

void CFrmSettings::openSlot() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open File"), QStringLiteral("/"), tr("Text file (*.*)"));
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("MDI"),
                           tr("Cannot read file %1:\n%2.")
                               .arg(fileName)
                               .arg(file.errorString()));
      return;
    }

    QTextStream in(&file);
    WIDGET(QTextEdit, teAttributes)->setPlainText(in.readAll());
  }
}

void CFrmSettings::saveSlot() {

  if (WIDGET(QTextEdit, teAttributes)->toPlainText().trimmed().isEmpty()) {
    QMessageBox::warning(this, tr("GvEdit"), tr("Nothing to save!"),
                         QMessageBox::Ok, QMessageBox::Ok);
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Open File"), QStringLiteral("/"), tr("Text File(*.*)"));
  if (!fileName.isEmpty()) {

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("MDI"),
                           tr("Cannot write file %1:\n%2.")
                               .arg(fileName)
                               .arg(file.errorString()));
      return;
    }

    QTextStream out(&file);
    out << WIDGET(QTextEdit, teAttributes)->toPlainText();
  }
}

bool CFrmSettings::loadGraph(MdiChild *m) {
  if (graph) {
    agclose(graph);
    graph = nullptr;
  }
  graphData.clear();
  graphData.append(m->toPlainText());
  setActiveWindow(m);
  return true;
}

bool CFrmSettings::createLayout() {
  rdr_t rdr;
  // first attach attributes to graph
  int _pos = graphData.indexOf(tr("{"));
  graphData.replace(_pos, 1,
                    QLatin1Char('{') +
                        WIDGET(QTextEdit, teAttributes)->toPlainText());

  QByteArray bytes = graphData.toUtf8();
  rdr.data = bytes.constData();
  rdr.len = strlen(rdr.data);
  rdr.cur = 0;
  graph = agmemread(rdr.data);
  if (!graph)
    return false;
  if (agerrors()) {
    agclose(graph);
    graph = nullptr;
    return false;
  }
  Agraph_t *G = this->graph;
  QString layout;

  layout = WIDGET(QComboBox, cbLayout)->currentText();

  gvLayout(gvc, G, layout.toUtf8().constData()); /* library function */
  return true;
}

static QString buildTempFile() {
  QTemporaryFile tempFile;
  tempFile.setAutoRemove(false);
  tempFile.open();
  QString a = tempFile.fileName();
  return a;
}

void CFrmSettings::doPreview(const QString &fileName) {
  if (getActiveWindow()->previewFrm != nullptr) {
    getActiveWindow()->parentFrm->mdiArea->removeSubWindow(
        getActiveWindow()->previewFrm->subWindowRef);
    getActiveWindow()->previewFrm = nullptr;
  }

  if (fileName.isNull() ||
      !getActiveWindow()->loadPreview(fileName)) { // create preview
    QString prevFile(buildTempFile());
    gvRenderFilename(gvc, graph, "png", prevFile.toUtf8().constData());
    getActiveWindow()->loadPreview(prevFile);
  }
}

bool CFrmSettings::renderLayout() {
  if (!graph)
    return false;
  QString sfx = WIDGET(QComboBox, cbExtension)->currentText();
  QString fileName(WIDGET(QLineEdit, leOutput)->text());

  if (fileName.isEmpty() || sfx == QLatin1String("NONE"))
    doPreview(QString());
  else {
    fileName = stripFileExtension(fileName);
    fileName = fileName + QLatin1Char('.') + sfx;
    if (fileName != activeWindow->outputFile)
      activeWindow->outputFile = fileName;

#ifdef _WIN32
    if (!fileName.contains(u'/') && !fileName.contains(u'\\'))
#else
    if (!fileName.contains(u'/'))
#endif
    { // no directory info => can we create/write the file?
      QFile outf(fileName);
      if (!outf.open(QIODevice::WriteOnly)) {
        QString pathName = QDir::homePath();
        pathName.append(u'/').append(fileName);
        fileName = QDir::toNativeSeparators(pathName);
        const QString msg =
            QStringLiteral("Output written to %1\n").arg(fileName);
        errorPipe(msg.toLatin1().data());
      }
    }

    if (gvRenderFilename(gvc, graph, sfx.toUtf8().constData(),
                         fileName.toUtf8().constData()))
      return false;

    doPreview(fileName);
  }
  return true;
}

bool CFrmSettings::loadLayouts() { return false; }

bool CFrmSettings::loadRenderers() { return false; }

void CFrmSettings::refreshContent() {

  WIDGET(QComboBox, cbLayout)->setCurrentIndex(activeWindow->layoutIdx);
  WIDGET(QComboBox, cbExtension)->setCurrentIndex(activeWindow->renderIdx);
  if (!activeWindow->outputFile.isEmpty())
    WIDGET(QLineEdit, leOutput)->setText(activeWindow->outputFile);
  else
    WIDGET(QLineEdit, leOutput)
        ->setText(stripFileExtension(activeWindow->currentFile()) +
                  QLatin1Char('.') +
                  WIDGET(QComboBox, cbExtension)->currentText());

  WIDGET(QTextEdit, teAttributes)->setText(activeWindow->attributes);

  WIDGET(QLineEdit, leValue)->clear();
}

void CFrmSettings::saveContent() {
  activeWindow->layoutIdx = WIDGET(QComboBox, cbLayout)->currentIndex();
  activeWindow->renderIdx = WIDGET(QComboBox, cbExtension)->currentIndex();
  activeWindow->outputFile = WIDGET(QLineEdit, leOutput)->text();
  activeWindow->attributes = WIDGET(QTextEdit, teAttributes)->toPlainText();
}

int CFrmSettings::drawGraph() {
  if (createLayout() && renderLayout()) {
    getActiveWindow()->settingsSet = false;
  }
  agreseterrors();

  return QDialog::Accepted;
}

int CFrmSettings::runSettings(MdiChild *m) {
  if (this->loadGraph(m))
    return drawGraph();

  if (m && m == getActiveWindow()) {
    if (this->loadGraph(m))
      return drawGraph();
    return QDialog::Rejected;
  }

  return showSettings(m);
}

int CFrmSettings::showSettings(MdiChild *m) {

  if (this->loadGraph(m)) {
    refreshContent();
    return this->exec();
  }
  return QDialog::Rejected;
}

void CFrmSettings::setActiveWindow(MdiChild *m) { this->activeWindow = m; }

MdiChild *CFrmSettings::getActiveWindow() { return activeWindow; }
