/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/main_window.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

#include <memory>  // for allocator, __shared_ptr
#include <string>  // for string, operator+, etc
#include <vector>  // for vector

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>

#ifdef OS_ANDROID
#include <QGestureEvent>
#endif

#include <common/convert2string.h>                // for ConvertFromString, etc
#include <common/error.h>                         // for Error, ErrnoErrorValue
#include <common/file_system.h>                   // for File, remove_file, etc
#include <common/macros.h>                        // for VERIFY, DNOTREACHED, CHECK, etc
#include <common/qt/convert2string.h>             // for ConvertToString
#include <common/qt/gui/app_style.h>              // for applyFont, applyStyle
#include <common/qt/gui/shortcuts.h>              // for FastoQKeySequence
#include <common/qt/logger.h>                     // for Logger
#include <common/qt/translations/translations.h>  // for applyLanguage
#include <common/text_decoders/iedcoder.h>        // for IEDcoder, EDTypes::Hex
#include <common/value.h>                         // for ErrorValue

#include "core/command/command_logger.h"      // for CommandLogger
#include "core/core_fwd.h"            // for IServerSPtr, IClusterSPtr, etc
#include "core/cluster/icluster.h"    // for ICluster
#include "core/sentinel/isentinel.h"  // for ISentinel
#include "core/servers_manager.h"     // for ServersManager
#include "core/settings_manager.h"    // for SettingsManager

#include "gui/dialogs/about_dialog.h"          // for AboutDialog
#include "gui/dialogs/connections_dialog.h"    // for ConnectionsDialog
#include "gui/dialogs/encode_decode_dialog.h"  // for EncodeDecodeDialog
#include "gui/dialogs/preferences_dialog.h"    // for PreferencesDialog
#include "gui/explorer/explorer_tree_view.h"   // for ExplorerTreeView
#include "gui/gui_factory.h"                   // for GuiFactory
#include "gui/shortcuts.h"                     // for fullScreenKey, openKey, etc
#include "gui/statistic_sender.h"              // for StatisticSender
#include "gui/update_checker.h"                // for UpdateChecker
#include "gui/widgets/log_tab_widget.h"        // for LogTabWidget
#include "gui/widgets/main_widget.h"           // for MainWidget

#include "translations/global.h"  // for trError, trCheckVersion, etc

namespace {

const QString trImportSettingsFailed = QObject::tr("Import settings failed!");
const QString trExportSettingsFailed = QObject::tr("Export settings failed!");
const QString trSettingsLoadedS = QObject::tr("Settings successfully loaded!");
const QString trSettingsImportedS = QObject::tr("Settings successfully imported!");
const QString trSettingsExportedS = QObject::tr("Settings successfully encrypted and exported!");

bool isNeededUpdate(const std::string& sversion) {
  uint32_t cver = common::ConvertVersionNumberFromString(sversion);
  return PROJECT_VERSION_NUMBER < cver;
}

const QKeySequence logsKeySequence = Qt::CTRL + Qt::Key_L;
const QKeySequence explorerKeySequence = Qt::CTRL + Qt::Key_T;

}  // namespace

namespace fastonosql {
namespace gui {

MainWindow::MainWindow() : QMainWindow(), isCheckedInSession_(false) {
#ifdef OS_ANDROID
  setAttribute(Qt::WA_AcceptTouchEvents);
  // setAttribute(Qt::WA_StaticContents);

  // grabGesture(Qt::TapGesture);  // click
  grabGesture(Qt::TapAndHoldGesture);  // long tap

// grabGesture(Qt::SwipeGesture);  // swipe
// grabGesture(Qt::PanGesture);  // drag and drop
// grabGesture(Qt::PinchGesture);  // zoom
#endif
  QString lang = core::SettingsManager::instance().currentLanguage();
  QString newLang = common::qt::translations::applyLanguage(lang);
  core::SettingsManager::instance().setCurrentLanguage(newLang);

  QString style = core::SettingsManager::instance().currentStyle();
  common::qt::gui::applyStyle(style);

  common::qt::gui::applyFont(gui::GuiFactory::instance().font());

  setWindowTitle(PROJECT_NAME_TITLE " " PROJECT_VERSION);

  openAction_ = new QAction(this);
  openAction_->setIcon(GuiFactory::instance().openIcon());
  openAction_->setShortcut(openKey);
  VERIFY(connect(openAction_, &QAction::triggered, this, &MainWindow::open));

  loadFromFileAction_ = new QAction(this);
  loadFromFileAction_->setIcon(GuiFactory::instance().loadIcon());
  // importAction_->setShortcut(openKey);
  VERIFY(connect(loadFromFileAction_, &QAction::triggered, this, &MainWindow::loadConnection));

  importAction_ = new QAction(this);
  importAction_->setIcon(GuiFactory::instance().importIcon());
  // importAction_->setShortcut(openKey);
  VERIFY(connect(importAction_, &QAction::triggered, this, &MainWindow::importConnection));

  exportAction_ = new QAction(this);
  exportAction_->setIcon(GuiFactory::instance().exportIcon());
  // exportAction_->setShortcut(openKey);
  VERIFY(connect(exportAction_, &QAction::triggered, this, &MainWindow::exportConnection));

  // Exit action
  exitAction_ = new QAction(this);
  exitAction_->setShortcut(quitKey);
  VERIFY(connect(exitAction_, &QAction::triggered, this, &MainWindow::close));

  // File menu
  QMenu* fileMenu = new QMenu(this);
  fileAction_ = menuBar()->addMenu(fileMenu);
  fileMenu->addAction(openAction_);
  fileMenu->addAction(loadFromFileAction_);
  fileMenu->addAction(importAction_);
  fileMenu->addAction(exportAction_);
  QMenu* recentMenu = new QMenu(this);
  recentConnections_ = fileMenu->addMenu(recentMenu);
  for (auto i = 0; i < max_recent_connections; ++i) {
    recentConnectionsActs_[i] = new QAction(this);
    VERIFY(connect(recentConnectionsActs_[i], &QAction::triggered, this,
                   &MainWindow::openRecentConnection));
    recentMenu->addAction(recentConnectionsActs_[i]);
  }

  clearMenu_ = new QAction(this);
  recentMenu->addSeparator();
  VERIFY(connect(clearMenu_, &QAction::triggered, this, &MainWindow::clearRecentConnectionsMenu));
  recentMenu->addAction(clearMenu_);

  fileMenu->addSeparator();
  fileMenu->addAction(exitAction_);
  updateRecentConnectionActions();

  preferencesAction_ = new QAction(this);
  preferencesAction_->setIcon(GuiFactory::instance().preferencesIcon());
  VERIFY(connect(preferencesAction_, &QAction::triggered, this, &MainWindow::openPreferences));

  QMenu* editMenu = new QMenu(this);
  editAction_ = menuBar()->addMenu(editMenu);
  editMenu->addAction(preferencesAction_);

  // tools menu
  QMenu* tools = new QMenu(this);
  toolsAction_ = menuBar()->addMenu(tools);

  encodeDecodeDialogAction_ = new QAction(this);
  encodeDecodeDialogAction_->setIcon(GuiFactory::instance().encodeDecodeIcon());
  VERIFY(connect(encodeDecodeDialogAction_, &QAction::triggered, this,
                 &MainWindow::openEncodeDecodeDialog));
  tools->addAction(encodeDecodeDialogAction_);

  // window menu
  QMenu* window = new QMenu(this);
  windowAction_ = menuBar()->addMenu(window);
  fullScreanAction_ = new QAction(this);
  fullScreanAction_->setShortcut(fullScreenKey);
  VERIFY(connect(fullScreanAction_, &QAction::triggered, this, &MainWindow::enterLeaveFullScreen));
  window->addAction(fullScreanAction_);

  QMenu* views = new QMenu(translations::trViews, this);
  window->addMenu(views);

  QMenu* helpMenu = new QMenu(this);
  aboutAction_ = new QAction(this);
  VERIFY(connect(aboutAction_, &QAction::triggered, this, &MainWindow::about));

  helpAction_ = menuBar()->addMenu(helpMenu);

  checkUpdateAction_ = new QAction(this);
  VERIFY(connect(checkUpdateAction_, &QAction::triggered, this, &MainWindow::checkUpdate));

  reportBugAction_ = new QAction(this);
  VERIFY(connect(reportBugAction_, &QAction::triggered, this, &MainWindow::reportBug));

  helpMenu->addAction(checkUpdateAction_);
  helpMenu->addSeparator();
  helpMenu->addAction(reportBugAction_);
  helpMenu->addSeparator();
  helpMenu->addAction(aboutAction_);

  MainWidget* mainW = new MainWidget;
  setCentralWidget(mainW);

  exp_ = new ExplorerTreeView(this);
  VERIFY(connect(exp_, &ExplorerTreeView::openedConsole, mainW, &MainWidget::openConsole));
  VERIFY(connect(exp_, &ExplorerTreeView::closeServer, &core::ServersManager::instance(),
                 &core::ServersManager::closeServer));
  VERIFY(connect(exp_, &ExplorerTreeView::closeCluster, &core::ServersManager::instance(),
                 &core::ServersManager::closeCluster));
  expDock_ = new QDockWidget(this);
  explorerAction_ = expDock_->toggleViewAction();
  explorerAction_->setShortcut(explorerKeySequence);
  explorerAction_->setChecked(true);
  views->addAction(explorerAction_);

  expDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                            Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  expDock_->setWidget(exp_);
  expDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  expDock_->setVisible(true);
  addDockWidget(Qt::LeftDockWidgetArea, expDock_);

  LogTabWidget* log = new LogTabWidget(this);
  VERIFY(connect(&common::qt::Logger::instance(), &common::qt::Logger::printed, log,
                 &LogTabWidget::addLogMessage));
  VERIFY(connect(&core::CommandLogger::instance(), &core::CommandLogger::printed, log,
                 &LogTabWidget::addCommand));
  logDock_ = new QDockWidget(this);
  logsAction_ = logDock_->toggleViewAction();
  logsAction_->setShortcut(logsKeySequence);
  logsAction_->setChecked(true);
  views->addAction(logsAction_);

  logDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                            Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  logDock_->setWidget(log);
  logDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  logDock_->setVisible(true);
  addDockWidget(Qt::BottomDockWidgetArea, logDock_);

  setMinimumSize(QSize(min_width, min_height));
  createToolBar();
  createStatusBar();
  retranslateUi();
}

MainWindow::~MainWindow() {
  core::ServersManager::instance().clear();
}

void MainWindow::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  return QMainWindow::changeEvent(ev);
}

void MainWindow::showEvent(QShowEvent* ev) {
  QMainWindow::showEvent(ev);
  bool isA = core::SettingsManager::instance().autoCheckUpdates();
  if (isA && !isCheckedInSession_) {
    isCheckedInSession_ = true;
    checkUpdate();
  }

  bool isSendedStatitic = core::SettingsManager::instance().isSendedStatistic();
  if (!isSendedStatitic) {
    sendStatistic();
  }
}

void MainWindow::open() {
  ConnectionsDialog dlg(this);
  int result = dlg.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  if (core::IConnectionSettingsBaseSPtr con = dlg.selectedConnection()) {
    createServer(con);
  } else if (core::IClusterSettingsBaseSPtr clus = dlg.selectedCluster()) {
    createCluster(clus);
  } else if (core::ISentinelSettingsBaseSPtr sent = dlg.selectedSentinel()) {
    createSentinel(sent);
  } else {
    NOTREACHED();
  }
}

void MainWindow::about() {
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::openPreferences() {
  PreferencesDialog dlg(this);
  dlg.exec();
}

void MainWindow::checkUpdate() {
  QThread* th = new QThread;
  UpdateChecker* cheker = new UpdateChecker;
  cheker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, cheker, &UpdateChecker::routine));
  VERIFY(connect(cheker, &UpdateChecker::versionAvailibled, this, &MainWindow::versionAvailible));
  VERIFY(connect(cheker, &UpdateChecker::versionAvailibled, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, cheker, &UpdateChecker::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

void MainWindow::sendStatistic() {
  QThread* th = new QThread;
  StatisticSender* sender = new StatisticSender;
  sender->moveToThread(th);
  VERIFY(connect(th, &QThread::started, sender, &StatisticSender::routine));
  VERIFY(connect(sender, &StatisticSender::statisticSended, this, &MainWindow::statitsticSent));
  VERIFY(connect(sender, &StatisticSender::statisticSended, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, sender, &UpdateChecker::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

void MainWindow::reportBug() {
  QDesktopServices::openUrl(QUrl(PROJECT_GITHUB_ISSUES));
}

void MainWindow::enterLeaveFullScreen() {
  if (isFullScreen()) {
    showNormal();
    fullScreanAction_->setText(translations::trEnterFullScreen);
  } else {
    showFullScreen();
    fullScreanAction_->setText(translations::trExitFullScreen);
  }
}

void MainWindow::openEncodeDecodeDialog() {
  EncodeDecodeDialog dlg(this);
  dlg.exec();
}

void MainWindow::openRecentConnection() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) {
    return;
  }

  QString rcon = action->text();
  std::string srcon = common::ConvertToString(rcon);
  core::ConnectionSettingsPath path(srcon);
  auto conns = core::SettingsManager::instance().connections();
  for (auto it = conns.begin(); it != conns.end(); ++it) {
    core::IConnectionSettingsBaseSPtr con = *it;
    if (con && con->path() == path) {
      createServer(con);
      return;
    }
  }
}

void MainWindow::loadConnection() {
  QString standardIni =
      common::ConvertFromString<QString>(core::SettingsManager::settingsFilePath());
  QString filepathR = QFileDialog::getOpenFileName(this, tr("Select settings file"), standardIni,
                                                   tr("Settings files (*.ini)"));
  if (filepathR.isNull()) {
    return;
  }

  core::SettingsManager::instance().reloadFromPath(common::ConvertToString(filepathR), false);
  QMessageBox::information(this, translations::trInfo, trSettingsLoadedS);
}

void MainWindow::importConnection() {
  std::string dir_path = core::SettingsManager::settingsDirPath();
  QString filepathR = QFileDialog::getOpenFileName(this, tr("Select encrypted settings file"),
                                                   common::ConvertFromString<QString>(dir_path),
                                                   tr("Encrypted settings files (*.cini)"));
  if (filepathR.isNull()) {
    return;
  }

  std::string tmp = core::SettingsManager::settingsFilePath() + ".tmp";

  common::file_system::ascii_string_path wp(tmp);
  common::file_system::File writeFile(wp);
  bool openedw = writeFile.open("wb");
  if (!openedw) {
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  common::file_system::ascii_string_path rp(common::ConvertToString(filepathR));
  common::file_system::File readFile(rp);
  bool openedr = readFile.open("rb");
  if (!openedr) {
    writeFile.close();
    common::Error err = common::file_system::remove_file(wp.path());
    if (err && err->isError()) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  common::IEDcoder* hexEnc = common::IEDcoder::createEDCoder(common::Hex);
  if (!hexEnc) {
    writeFile.close();
    common::Error err = common::file_system::remove_file(wp.path());
    if (err && err->isError()) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  while (!readFile.isEof()) {
    std::string data;
    bool res = readFile.read(&data, 256);
    if (!res) {
      break;
    }

    std::string edata;
    common::Error er = hexEnc->decode(data, &edata);
    if (er && er->isError()) {
      writeFile.close();
      common::Error err = common::file_system::remove_file(wp.path());
      if (err && err->isError()) {
        DNOTREACHED();
      }
      QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
      return;
    } else {
      writeFile.write(edata);
    }
  }

  writeFile.close();
  core::SettingsManager::instance().reloadFromPath(tmp, false);
  common::Error err = common::file_system::remove_file(tmp);
  if (err && err->isError()) {
    DNOTREACHED();
  }
  QMessageBox::information(this, translations::trInfo, trSettingsImportedS);
}

void MainWindow::exportConnection() {
  std::string dir_path = core::SettingsManager::settingsDirPath();
  QString filepathW = QFileDialog::getSaveFileName(this, tr("Select file to save settings"),
                                                   common::ConvertFromString<QString>(dir_path),
                                                   tr("Settings files (*.cini)"));
  if (filepathW.isEmpty()) {
    return;
  }

  common::file_system::ascii_string_path wp(common::ConvertToString(filepathW));
  common::file_system::File writeFile(wp);
  bool openedw = writeFile.open("wb");
  if (!openedw) {
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  common::file_system::ascii_string_path rp(core::SettingsManager::settingsFilePath());
  common::file_system::File readFile(rp);
  bool openedr = readFile.open("rb");
  if (!openedr) {
    writeFile.close();
    common::Error err = common::file_system::remove_file(wp.path());
    if (err && err->isError()) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  common::IEDcoder* hexEnc = common::IEDcoder::createEDCoder(common::Hex);
  if (!hexEnc) {
    writeFile.close();
    common::Error err = common::file_system::remove_file(wp.path());
    if (err && err->isError()) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  while (!readFile.isEof()) {
    std::string data;
    bool res = readFile.readLine(&data);
    if (!res || readFile.isEof()) {
      break;
    }

    std::string edata;
    common::Error er = hexEnc->encode(data, &edata);
    if (er && er->isError()) {
      writeFile.close();
      common::Error err = common::file_system::remove_file(wp.path());
      if (err && err->isError()) {
        DNOTREACHED();
      }
      QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
      return;
    } else {
      writeFile.write(edata);
    }
  }

  QMessageBox::information(this, translations::trInfo, trSettingsExportedS);
}

void MainWindow::versionAvailible(bool succesResult, const QString& version) {
  if (!succesResult) {
    QMessageBox::information(this, translations::trCheckVersion,
                             translations::trConnectionErrorText);
    checkUpdateAction_->setEnabled(true);
    return;
  }

  std::string sver = common::ConvertToString(version);
  bool isn = isNeededUpdate(sver);
  if (isn) {
    QMessageBox::information(this, translations::trCheckVersion,
                             QObject::tr("Availible new version: %1").arg(version));
  } else {
    QMessageBox::information(
        this, translations::trCheckVersion,
        QObject::tr("<h3>You're' "
                    "up-to-date!</h3>" PROJECT_NAME_TITLE " %1 is currently the newest version "
                    "available.")
            .arg(version));
  }

  checkUpdateAction_->setEnabled(isn);
}

void MainWindow::statitsticSent(bool succesResult) {
  if (succesResult) {
    core::SettingsManager::instance().setIsSendedStatistic(true);
  }
}

#ifdef OS_ANDROID
bool MainWindow::event(QEvent* event) {
  if (event->type() == QEvent::Gesture) {
    QGestureEvent* gest = static_cast<QGestureEvent*>(event);
    if (gest) {
      return gestureEvent(gest);
    }
  }
  return QMainWindow::event(event);
}

bool MainWindow::gestureEvent(QGestureEvent* event) {
  /*if (QGesture* qpan = event->gesture(Qt::PanGesture)){
    QPanGesture* pan = static_cast<QPanGesture*>(qpan);
  }
  if (QGesture* qpinch = event->gesture(Qt::PinchGesture)){
    QPinchGesture* pinch =
  static_cast<QPinchGesture*>(qpinch);
  }
  if (QGesture* qtap = event->gesture(Qt::TapGesture)){
    QTapGesture* tap = static_cast<QTapGesture*>(qtap);
  }*/

  if (QGesture* qswipe = event->gesture(Qt::SwipeGesture)) {
    QSwipeGesture* swipe = static_cast<QSwipeGesture*>(qswipe);
    swipeTriggered(swipe);
  } else if (QGesture* qtapandhold = event->gesture(Qt::TapAndHoldGesture)) {
    QTapAndHoldGesture* tapandhold = static_cast<QTapAndHoldGesture*>(qtapandhold);
    tapAndHoldTriggered(tapandhold);
    event->accept();
  }

  return true;
}

void MainWindow::swipeTriggered(QSwipeGesture* swipeEvent) {}

void MainWindow::tapAndHoldTriggered(QTapAndHoldGesture* tapEvent) {
  QPoint pos = tapEvent->position().toPoint();
  QContextMenuEvent* cont = new QContextMenuEvent(QContextMenuEvent::Mouse, pos, mapToGlobal(pos));
  QWidget* rec = childAt(pos);
  QApplication::postEvent(rec, cont);
}
#endif

void MainWindow::createToolBar() {
#ifdef BUILD_WITH_SOCIAL_BUTTONS
  QToolBar* toolBar = new QToolBar(tr("Share toolbar"));

  facebookAction_ = new QAction(this);
  facebookAction_->setIcon(GuiFactory::instance().facebookIcon());
  VERIFY(connect(facebookAction_, &QAction::triggered, this, &MainWindow::openFacebookLink));

  twitterAction_ = new QAction(this);
  twitterAction_->setIcon(GuiFactory::instance().twitterIcon());
  VERIFY(connect(twitterAction_, &QAction::triggered, this, &MainWindow::openTwitterLink));

  githubAction_ = new QAction(this);
  githubAction_->setIcon(GuiFactory::instance().githubIcon());
  VERIFY(connect(githubAction_, &QAction::triggered, this, &MainWindow::openGithubLink));

  homePageAction_ = new QAction(this);
  homePageAction_->setIcon(GuiFactory::instance().homePageIcon());
  VERIFY(connect(homePageAction_, &QAction::triggered, this, &MainWindow::openHomePageLink));

  toolBar->addAction(homePageAction_);
  toolBar->addAction(facebookAction_);
  toolBar->addAction(twitterAction_);
  toolBar->addAction(githubAction_);

  toolBar->setMovable(false);
  addToolBar(Qt::TopToolBarArea, toolBar);
#endif
}

void MainWindow::openHomePageLink() {
  QDesktopServices::openUrl(QUrl(PROJECT_HOMEPAGE_LINK));
}

void MainWindow::openFacebookLink() {
  QDesktopServices::openUrl(QUrl(PROJECT_FACEBOOK_LINK));
}

void MainWindow::openTwitterLink() {
  QDesktopServices::openUrl(QUrl(PROJECT_TWITTER_LINK));
}

void MainWindow::openGithubLink() {
  QDesktopServices::openUrl(QUrl(PROJECT_GITHUB_LINK));
}

void MainWindow::createStatusBar() {}

void MainWindow::retranslateUi() {
  openAction_->setText(translations::trOpen);
  loadFromFileAction_->setText(translations::trLoadFromFile);
  importAction_->setText(translations::trImport);
  exportAction_->setText(translations::trExport);
  exitAction_->setText(translations::trExit);
  fileAction_->setText(translations::trFile);
  toolsAction_->setText(translations::trTools);
  encodeDecodeDialogAction_->setText(translations::trEncodeDecode);
  preferencesAction_->setText(translations::trPreferences);
  checkUpdateAction_->setText(translations::trCheckUpdate);
  editAction_->setText(translations::trEdit);
  windowAction_->setText(translations::trWindow);
  fullScreanAction_->setText(translations::trEnterFullScreen);
  reportBugAction_->setText(translations::trReportBug);
  aboutAction_->setText(tr("About %1...").arg(PROJECT_NAME_TITLE));
  helpAction_->setText(translations::trHelp);
  explorerAction_->setText(translations::trExpTree);
  logsAction_->setText(translations::trLogs);
  recentConnections_->setText(translations::trRecentConnections);
  clearMenu_->setText(translations::trClearMenu);
#ifdef BUILD_WITH_SOCIAL_BUTTONS
  homePageAction_->setText(tr("%1 home page").arg(PROJECT_NAME_TITLE));
  facebookAction_->setText(tr("Facebook %1 link").arg(PROJECT_NAME_TITLE));
  twitterAction_->setText(tr("Twitter %1 link").arg(PROJECT_NAME_TITLE));
  githubAction_->setText(tr("Github %1 link").arg(PROJECT_NAME_TITLE));
#endif
  expDock_->setWindowTitle(translations::trExpTree);
  logDock_->setWindowTitle(translations::trLogs);
}

void MainWindow::updateRecentConnectionActions() {
  QStringList connections = core::SettingsManager::instance().recentConnections();

  int numRecentFiles = qMin(connections.size(), static_cast<int>(max_recent_connections));

  for (int i = 0; i < numRecentFiles; ++i) {
    QString text = connections[i];
    recentConnectionsActs_[i]->setText(text);
    recentConnectionsActs_[i]->setVisible(true);
  }

  for (int j = numRecentFiles; j < max_recent_connections; ++j) {
    recentConnectionsActs_[j]->setVisible(false);
  }

  bool isHaveItem = numRecentFiles > 0;
  clearMenu_->setVisible(isHaveItem);
  recentConnections_->setEnabled(isHaveItem);
}

void MainWindow::clearRecentConnectionsMenu() {
  core::SettingsManager::instance().clearRConnections();
  updateRecentConnectionActions();
}

void MainWindow::createServer(core::IConnectionSettingsBaseSPtr settings) {
  CHECK(settings);

  std::string path = settings->path().toString();
  QString rcon = common::ConvertFromString<QString>(path);
  core::SettingsManager::instance().removeRConnection(rcon);
  core::IServerSPtr server = core::ServersManager::instance().createServer(settings);
  exp_->addServer(server);
  core::SettingsManager::instance().addRConnection(rcon);
  updateRecentConnectionActions();
  if (!core::SettingsManager::instance().autoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    mwidg->openConsole(server, QString());
  }
}

void MainWindow::createSentinel(core::ISentinelSettingsBaseSPtr settings) {
  CHECK(settings);

  core::ISentinelSPtr sent = core::ServersManager::instance().createSentinel(settings);
  if (!sent) {
    return;
  }

  auto sentinels = sent->sentinels();
  if (sentinels.empty()) {
    return;
  }

  exp_->addSentinel(sent);
  if (!core::SettingsManager::instance().autoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    for (size_t i = 0; i < sentinels.size(); ++i) {
      core::IServerSPtr serv = sentinels[i].sentinel;
      mwidg->openConsole(serv, QString());
    }
  }
}

void MainWindow::createCluster(core::IClusterSettingsBaseSPtr settings) {
  CHECK(settings);

  core::IClusterSPtr cl = core::ServersManager::instance().createCluster(settings);
  if (!cl) {
    return;
  }

  core::IServerSPtr root = cl->root();
  if (!root) {
    return;
  }

  exp_->addCluster(cl);
  if (!core::SettingsManager::instance().autoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    mwidg->openConsole(root, QString());
  }
}

}  // namespace gui
}  // namespace fastonosql
