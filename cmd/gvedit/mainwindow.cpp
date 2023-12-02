/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "mainwindow.h"
#include "config.h"
#include "csettings.h"
#include "mdichild.h"
#include <QStringList>
#include <QtWidgets>
#include <optional>
#include <qframe.h>
#include <string_view>

QTextEdit *globTextEdit;

int errorPipe(char *errMsg) {
  globTextEdit->setText(globTextEdit->toPlainText() +
                        QString::fromUtf8(errMsg));
  return 0;
}

static void freeList(char **lp, int count) {
  for (int i = 0; i < count; i++)
    free(lp[i]);
  free(lp);
}

static int LoadPlugins(QComboBox &cb, GVC_t *gvc, const char *kind,
                       const QStringList &more, std::string_view prefer) {
  int count;
  char **lp = gvPluginList(gvc, kind, &count);
  std::optional<int> idx;

  cb.clear();
  for (int id = 0; id < count; id++) {
    cb.addItem(QString::fromUtf8(lp[id]));
    if (!idx.has_value() && prefer == lp[id])
      idx = id;
  }
  freeList(lp, count);

  /* Add additional items if supplied */
  cb.addItems(more);

  if (idx.has_value())
    cb.setCurrentIndex(*idx);

  return idx.value_or(0);
}

void CMainWindow::createConsole() {
  QDockWidget *dock = new QDockWidget(tr("Output Console"), nullptr);
  QTextEdit *textEdit = new QTextEdit(dock);

  dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  addDockWidget(Qt::BottomDockWidgetArea, dock);
  QVBoxLayout *vL = new QVBoxLayout();

  textEdit->setObjectName(QStringLiteral("textEdit"));
  globTextEdit = textEdit;
  agseterrf(errorPipe);

  vL->addWidget(textEdit);
  vL->setContentsMargins(1, 1, 1, 1);

  QFrame *fr = new QFrame(dock);
  vL->addWidget(fr);

  QPushButton *logNewBtn =
      new QPushButton(QIcon(QStringLiteral(":/images/new.png")), {}, fr);
  QPushButton *logSaveBtn =
      new QPushButton(QIcon(QStringLiteral(":/images/save.png")), {}, fr);
  QHBoxLayout *consoleLayout = new QHBoxLayout();
  consoleLayout->addWidget(logNewBtn);
  connect(logNewBtn, &QPushButton::clicked, this, &CMainWindow::slotNewLog);
  connect(logSaveBtn, &QPushButton::clicked, this, &CMainWindow::slotSaveLog);
  consoleLayout->addWidget(logSaveBtn);
  consoleLayout->addStretch();

  consoleLayout->setContentsMargins(1, 1, 1, 1);

  fr->setLayout(consoleLayout);

  QFrame *mainFrame = new QFrame(dock);
  mainFrame->setLayout(vL);

  dock->setWidget(mainFrame);
}

static const QStringList xtra = {QStringLiteral("NONE")};

CMainWindow::CMainWindow(const QStringList &files) {

  QWidget *centralwidget = new QWidget(this);
  centralwidget->setObjectName(QStringLiteral("centralwidget"));
  QVBoxLayout *verticalLayout_2 = new QVBoxLayout(centralwidget);
  verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
  QVBoxLayout *verticalLayout = new QVBoxLayout();
  verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
  mdiArea = new QMdiArea(centralwidget);
  mdiArea->setObjectName(QStringLiteral("mdiArea"));

  verticalLayout->addWidget(mdiArea);
  verticalLayout_2->setContentsMargins(1, 1, 1, 1);
  verticalLayout_2->addLayout(verticalLayout);
  setCentralWidget(centralwidget);
  centralwidget->layout()->setContentsMargins(1, 1, 1, 1);
  prevChild = nullptr;

  createConsole();

  connect(mdiArea, &QMdiArea::subWindowActivated, this,
          &CMainWindow::slotRefreshMenus);

  frmSettings = new CFrmSettings();

  actions();
  menus();
  toolBars();
  statusBar();
  updateMenus();

  readSettings();

  setWindowTitle(tr("GVEdit"));
  this->resize(1024, 900);
  this->move(0, 0);
  setUnifiedTitleAndToolBarOnMac(true);
  QComboBox *cb =
      frmSettings->findChild<QComboBox *>(QStringLiteral("cbLayout"));
  dfltLayoutIdx = LoadPlugins(*cb, frmSettings->gvc, "layout", {}, "dot");
  cb = frmSettings->findChild<QComboBox *>(QStringLiteral("cbExtension"));
  dfltRenderIdx = LoadPlugins(*cb, frmSettings->gvc, "device", xtra, "png");
  statusBar()->showMessage(tr("Ready"));
  setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));
  // load files specified in command line , one time task
  for (const QString &file : files) {
    addFile(file);
  }
}

void CMainWindow::closeEvent(QCloseEvent *event) {
  mdiArea->closeAllSubWindows();
  if (mdiArea->currentSubWindow()) {
    event->ignore();
  } else {
    writeSettings();
    event->accept();
  }
}

void CMainWindow::slotNew() {
  MdiChild *child = createMdiChild();
  child->newFile();
  child->show();
}

void CMainWindow::addFile(const QString &fileName) {
  if (!fileName.isEmpty()) {
    QMdiSubWindow *existing = findMdiChild(fileName);
    if (existing) {
      mdiArea->setActiveSubWindow(existing);
      return;
    }

    MdiChild *child = createMdiChild();
    if (child->loadFile(fileName)) {
      statusBar()->showMessage(tr("File loaded"), 2000);
      child->show();
      slotRun(child);
    } else {
      child->close();
    }
  }
}

void CMainWindow::slotOpen() {
  QStringList filters{
      QStringLiteral("*.cpp"),
      QStringLiteral("*.cxx"),
      QStringLiteral("*.cc"),
  };

  QFileDialog fd;
  fd.setNameFilter(QStringLiteral("XML (*.xml)"));
  QString fileName = fd.getOpenFileName(this);

  addFile(fileName);
}

void CMainWindow::slotSave() {
  if (activeMdiChild() && activeMdiChild()->save())
    statusBar()->showMessage(tr("File saved"), 2000);
}

void CMainWindow::slotSaveAs() {
  if (activeMdiChild() && activeMdiChild()->saveAs())
    statusBar()->showMessage(tr("File saved"), 2000);
}

void CMainWindow::slotCut() {
  if (activeMdiChild())
    activeMdiChild()->cut();
}

void CMainWindow::slotCopy() {
  if (activeMdiChild())
    activeMdiChild()->copy();
}

void CMainWindow::slotPaste() {
  if (activeMdiChild())
    activeMdiChild()->paste();
}

void CMainWindow::slotAbout() {
  QString abs(tr("<b>GVEdit</b> Graph File Editor For Graphviz"
                 " version: 1.02\n"
                 "Graphviz version: "));

  abs += tr(gvcVersion(frmSettings->gvc));
  QMessageBox::about(this, tr("About GVEdit"), abs);
}

void CMainWindow::setChild() {
  if (prevChild != activeMdiChild()) {
    const QString msg =
        QStringLiteral("working on %1\n").arg(activeMdiChild()->currentFile());
    errorPipe(msg.toLatin1().data());
    prevChild = activeMdiChild();
  }
}

void CMainWindow::slotSettings() {
  setChild();
  frmSettings->showSettings(activeMdiChild());
}

void CMainWindow::slotRun(MdiChild *m) {
  setChild();

  if (m)
    frmSettings->runSettings(m);
  else
    frmSettings->runSettings(activeMdiChild());
}

void CMainWindow::slotNewLog() { globTextEdit->clear(); }

void CMainWindow::slotSaveLog() {

  if (globTextEdit->toPlainText().trimmed().isEmpty()) {
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
    out << globTextEdit->toPlainText();
  }
}

void CMainWindow::updateFileMenu() {
  if (!activeMdiChild()) {
    saveAct->setEnabled(false);
    saveAsAct->setEnabled(false);
    pasteAct->setEnabled(false);
    closeAct->setEnabled(false);
    closeAllAct->setEnabled(false);
    tileAct->setEnabled(false);
    cascadeAct->setEnabled(false);
    nextAct->setEnabled(false);
    previousAct->setEnabled(false);
    separatorAct->setVisible(false);
    settingsAct->setEnabled(false);
    layoutAct->setEnabled(false);
  } else {
    saveAct->setEnabled(true);
    saveAsAct->setEnabled(true);
    pasteAct->setEnabled(true);
    closeAct->setEnabled(true);
    closeAllAct->setEnabled(true);
    tileAct->setEnabled(true);
    cascadeAct->setEnabled(true);
    nextAct->setEnabled(true);
    previousAct->setEnabled(true);
    separatorAct->setVisible(true);
    settingsAct->setEnabled(true);
    layoutAct->setEnabled(true);

    if (activeMdiChild()->textCursor().hasSelection()) {
      cutAct->setEnabled(true);
      copyAct->setEnabled(true);

    } else {
      cutAct->setEnabled(false);
      copyAct->setEnabled(false);
    }
  }
}

void CMainWindow::slotRefreshMenus() { updateMenus(); }

void CMainWindow::updateMenus() {
  this->updateFileMenu();
  this->updateWindowMenu();
}

void CMainWindow::updateWindowMenu() {
  mWindow->clear();
  mWindow->addAction(closeAct);
  mWindow->addAction(closeAllAct);
  mWindow->addSeparator();
  mWindow->addAction(tileAct);
  mWindow->addAction(cascadeAct);
  mWindow->addSeparator();
  mWindow->addAction(nextAct);
  mWindow->addAction(previousAct);
  mWindow->addAction(separatorAct);

  QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
  separatorAct->setVisible(!windows.isEmpty());

  for (int i = 0; i < windows.size(); ++i) {
    QMdiSubWindow *window = windows.at(i);
    if (window->widget()->inherits("MdiChild")) {
      MdiChild *child = qobject_cast<MdiChild *>(window->widget());
      QString text;
      if (i < 9) {
        text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
      } else {
        text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
      }
      QAction *action = mWindow->addAction(text);
      action->setCheckable(true);
      action->setChecked(child == activeMdiChild());
      connect(action, &QAction::triggered, this,
              [this, window] { activateChild(window); });
    }
  }
}

MdiChild *CMainWindow::createMdiChild() {
  MdiChild *child = new MdiChild;
  child->parentFrm = this;
  QMdiSubWindow *s = mdiArea->addSubWindow(child);
  s->resize(800, 600);
  s->move(mdiArea->subWindowList().count() * 5,
          mdiArea->subWindowList().count() * 5);
  connect(child, &MdiChild::copyAvailable, cutAct, &QAction::setEnabled);
  connect(child, &MdiChild::copyAvailable, copyAct, &QAction::setEnabled);
  child->layoutIdx = dfltLayoutIdx;
  child->renderIdx = dfltRenderIdx;

  return child;
}

void CMainWindow::actions() {
  newAct =
      new QAction(QIcon(QStringLiteral(":/images/new.png")), tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &CMainWindow::slotNew);

  openAct = new QAction(QIcon(QStringLiteral(":/images/open.png")),
                        tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &CMainWindow::slotOpen);

  saveAct = new QAction(QIcon(QStringLiteral(":/images/save.png")), tr("&Save"),
                        this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, &QAction::triggered, this, &CMainWindow::slotSave);

  saveAsAct = new QAction(tr("Save &As..."), this);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, &QAction::triggered, this, &CMainWindow::slotSaveAs);

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, &QAction::triggered, qApp, &QApplication::closeAllWindows);

  cutAct =
      new QAction(QIcon(QStringLiteral(":/images/cut.png")), tr("Cu&t"), this);
  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                          "clipboard"));
  connect(cutAct, &QAction::triggered, this, &CMainWindow::slotCut);

  copyAct = new QAction(QIcon(QStringLiteral(":/images/copy.png")), tr("&Copy"),
                        this);
  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                           "clipboard"));
  connect(copyAct, &QAction::triggered, this, &CMainWindow::slotCopy);

  pasteAct = new QAction(QIcon(QStringLiteral(":/images/paste.png")),
                         tr("&Paste"), this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                            "selection"));
  connect(pasteAct, &QAction::triggered, this, &CMainWindow::slotPaste);

  closeAct = new QAction(tr("Cl&ose"), this);
  closeAct->setStatusTip(tr("Close the active window"));
  connect(closeAct, &QAction::triggered, mdiArea,
          &QMdiArea::closeActiveSubWindow);

  closeAllAct = new QAction(tr("Close &All"), this);
  closeAllAct->setStatusTip(tr("Close all the windows"));
  connect(closeAllAct, &QAction::triggered, mdiArea,
          &QMdiArea::closeAllSubWindows);

  tileAct = new QAction(tr("&Tile"), this);
  tileAct->setStatusTip(tr("Tile the windows"));
  connect(tileAct, &QAction::triggered, mdiArea, &QMdiArea::tileSubWindows);

  cascadeAct = new QAction(tr("&Cascade"), this);
  cascadeAct->setStatusTip(tr("Cascade the windows"));
  connect(cascadeAct, &QAction::triggered, mdiArea,
          &QMdiArea::cascadeSubWindows);

  nextAct = new QAction(tr("Ne&xt"), this);
  nextAct->setShortcuts(QKeySequence::NextChild);
  nextAct->setStatusTip(tr("Move the focus to the next window"));
  connect(nextAct, &QAction::triggered, mdiArea,
          &QMdiArea::activateNextSubWindow);

  previousAct = new QAction(tr("Pre&vious"), this);
  previousAct->setShortcuts(QKeySequence::PreviousChild);
  previousAct->setStatusTip(tr("Move the focus to the previous window"));
  connect(previousAct, &QAction::triggered, mdiArea,
          &QMdiArea::activatePreviousSubWindow);

  separatorAct = new QAction(this);
  separatorAct->setSeparator(true);

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, &QAction::triggered, this, &CMainWindow::slotAbout);

  settingsAct = new QAction(QIcon(QStringLiteral(":/images/settings.png")),
                            tr("Settings"), this);
  settingsAct->setStatusTip(tr("Show Graphviz Settings"));
  connect(settingsAct, &QAction::triggered, this, &CMainWindow::slotSettings);
  settingsAct->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));

  layoutAct = new QAction(QIcon(QStringLiteral(":/images/run.png")),
                          tr("Layout"), this);
  layoutAct->setStatusTip(tr("Layout the active graph"));
  connect(layoutAct, &QAction::triggered, this, [this] { slotRun(); });
  layoutAct->setShortcut(QKeySequence(Qt::Key_F5));
}

void CMainWindow::menus() {
  mFile = menuBar()->addMenu(tr("&File"));
  mEdit = menuBar()->addMenu(tr("&Edit"));
  mWindow = menuBar()->addMenu(tr("&Window"));
  mGraph = menuBar()->addMenu(tr("&Graph"));
  mHelp = menuBar()->addMenu(tr("&Help"));

  mFile->addAction(newAct);
  mFile->addAction(openAct);
  mFile->addAction(saveAct);
  mFile->addAction(saveAsAct);
  mFile->addSeparator();

  mFile->addAction(exitAct);

  mEdit->addAction(cutAct);
  mEdit->addAction(copyAct);
  mEdit->addAction(pasteAct);

  mGraph->addAction(settingsAct);
  mGraph->addAction(layoutAct);
  mGraph->addSeparator();

  updateWindowMenu();
  connect(mWindow, &QMenu::aboutToShow, this, &CMainWindow::slotRefreshMenus);
  mHelp->addAction(aboutAct);
}

void CMainWindow::toolBars() {
  tbFile = addToolBar(tr("File"));
  tbFile->addAction(newAct);
  tbFile->addAction(openAct);
  tbFile->addAction(saveAct);

  tbEdit = addToolBar(tr("Edit"));
  tbEdit->addAction(cutAct);
  tbEdit->addAction(copyAct);
  tbEdit->addAction(pasteAct);

  tbGraph = addToolBar(tr("Graph"));
  tbGraph->addAction(settingsAct);
  tbGraph->addAction(layoutAct);
}

void CMainWindow::readSettings() {
  // first try new settings
  {
    QSettings settings(QStringLiteral("Graphviz"), QStringLiteral("gvedit"));
    if (settings.contains(QStringLiteral("pos")) &&
        settings.contains(QStringLiteral("size"))) {
      QPoint pos =
          settings.value(QStringLiteral("pos"), QPoint(200, 200)).toPoint();
      QSize size =
          settings.value(QStringLiteral("size"), QSize(400, 400)).toSize();
      move(pos);
      resize(size);
      return;
    }
  }

  // fall back to old settings
  QSettings settings(QStringLiteral("Trolltech"),
                     QStringLiteral("MDI Example"));
  QPoint pos =
      settings.value(QStringLiteral("pos"), QPoint(200, 200)).toPoint();
  QSize size = settings.value(QStringLiteral("size"), QSize(400, 400)).toSize();
  move(pos);
  resize(size);
}

void CMainWindow::writeSettings() {
  QSettings settings(QStringLiteral("Graphviz"), QStringLiteral("gvedit"));
  settings.setValue(QStringLiteral("pos"), pos());
  settings.setValue(QStringLiteral("size"), size());
}

MdiChild *CMainWindow::activeMdiChild() {
  if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow()) {
    if (activeSubWindow->widget()->inherits("MdiChild"))
      return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return qobject_cast<ImageViewer *>(activeSubWindow->widget())->graphWindow;
  }
  return 0;
}

QMdiSubWindow *CMainWindow::findMdiChild(const QString &fileName) {
  QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

  foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
    if (window->widget()->inherits("MdiChild")) {

      MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
      if (mdiChild->currentFile() == canonicalFilePath)
        return window;
    } else {

      MdiChild *mdiChild =
          qobject_cast<ImageViewer *>(window->widget())->graphWindow;
      if (mdiChild->currentFile() == canonicalFilePath)
        return window;
    }
  }
  return 0;
}

void CMainWindow::activateChild(QWidget *window) {
  if (!window)
    return;
  mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}
