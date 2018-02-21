/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <QAction>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QUrl>

#ifdef OS_ANDROID
#include <QApplication>
#include <QGestureEvent>
#endif

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/text_decoders/iedcoder_factory.h>  // for IEDcoder, EDTypes::Hex

#include <common/qt/convert2string.h>             // for ConvertToString
#include <common/qt/gui/app_style.h>              // for applyFont, applyStyle
#include <common/qt/logger.h>                     // for Logger
#include <common/qt/translations/translations.h>  // for applyLanguage

#include "core/logger.h"
#include "core/ssh_info.h"

#include "proxy/cluster/icluster.h"        // for ICluster
#include "proxy/command/command_logger.h"  // for CommandLogger
#include "proxy/sentinel/isentinel.h"      // for ISentinel
#include "proxy/server/iserver.h"
#include "proxy/servers_manager.h"   // for ServersManager
#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/dialogs/about_dialog.h"          // for AboutDialog
#include "gui/dialogs/connections_dialog.h"    // for ConnectionsDialog
#include "gui/dialogs/encode_decode_dialog.h"  // for EncodeDecodeDialog
#include "gui/dialogs/how_to_use_dialog.h"
#include "gui/dialogs/preferences_dialog.h"     // for PreferencesDialog
#include "gui/explorer/explorer_tree_widget.h"  // for ExplorerTreeWidget
#include "gui/gui_factory.h"                    // for GuiFactory
#include "gui/shortcuts.h"                      // for g_full_screen_key, g_open_key, etc
#include "gui/statistic_sender.h"               // for StatisticSender
#include "gui/update_checker.h"                 // for UpdateChecker
#include "gui/utils.h"
#include "gui/widgets/log_tab_widget.h"  // for LogTabWidget
#include "gui/widgets/main_widget.h"     // for MainWidget

#include "translations/global.h"  // for trError, trCheckVersion, etc

namespace {

const QString trImportSettingsFailed = QObject::tr("Import settings failed!");
const QString trExportSettingsFailed = QObject::tr("Export settings failed!");
const QString trSettingsLoadedS = QObject::tr("Settings successfully loaded!");
const QString trSettingsImportedS = QObject::tr("Settings successfully imported!");
const QString trSettingsExportedS = QObject::tr("Settings successfully encrypted and exported!");

bool IsNeededUpdate(const std::string& sversion) {
  uint32_t cver = common::ConvertVersionNumberFromString(sversion);
  return PROJECT_VERSION_NUMBER < cver;
}

const QKeySequence logsKeySequence = Qt::CTRL + Qt::Key_L;
const QKeySequence explorerKeySequence = Qt::CTRL + Qt::Key_T;

void LogWatcherRedirect(common::logging::LOG_LEVEL level, const std::string& message, bool notify) {
  LOG_MSG(message, level, notify);
}

}  // namespace

namespace fastonosql {
namespace gui {

MainWindow::MainWindow() : QMainWindow() {
#ifdef OS_ANDROID
  setAttribute(Qt::WA_AcceptTouchEvents);
  // setAttribute(Qt::WA_StaticContents);

  // grabGesture(Qt::TapGesture);  // click
  grabGesture(Qt::TapAndHoldGesture);  // long tap

// grabGesture(Qt::SwipeGesture);  // swipe
// grabGesture(Qt::PanGesture);  // drag and drop
// grabGesture(Qt::PinchGesture);  // zoom
#endif
  QString lang = proxy::SettingsManager::GetInstance()->GetCurrentLanguage();
  QString newLang = common::qt::translations::applyLanguage(lang);
  proxy::SettingsManager::GetInstance()->SetCurrentLanguage(newLang);

  QString style = proxy::SettingsManager::GetInstance()->GetCurrentStyle();
  common::qt::gui::applyStyle(style);

  common::qt::gui::applyFont(gui::GuiFactory::GetInstance().GetFont());

  setWindowTitle(PROJECT_NAME_TITLE " " PROJECT_VERSION);

  connect_action_ = new QAction(this);
  connect_action_->setIcon(GuiFactory::GetInstance().GetConnectDBIcon());
  connect_action_->setShortcut(g_new_key);
  VERIFY(connect(connect_action_, &QAction::triggered, this, &MainWindow::open));

  load_from_file_action_ = new QAction(this);
  load_from_file_action_->setIcon(GuiFactory::GetInstance().GetLoadIcon());
  // import_action_->setShortcut(g_open_key);
  VERIFY(connect(load_from_file_action_, &QAction::triggered, this, &MainWindow::loadConnection));

  import_action_ = new QAction(this);
  import_action_->setIcon(GuiFactory::GetInstance().GetImportIcon());
  // import_action_->setShortcut(g_open_key);
  VERIFY(connect(import_action_, &QAction::triggered, this, &MainWindow::importConnection));

  export_action_ = new QAction(this);
  export_action_->setIcon(GuiFactory::GetInstance().GetExportIcon());
  // export_action_->setShortcut(g_open_key);
  VERIFY(connect(export_action_, &QAction::triggered, this, &MainWindow::exportConnection));

  // Exit action
  exit_action_ = new QAction(this);
  exit_action_->setShortcut(g_quit_key);
  VERIFY(connect(exit_action_, &QAction::triggered, this, &MainWindow::close));

  // File menu
  QMenu* fileMenu = new QMenu(this);
  file_action_ = menuBar()->addMenu(fileMenu);
  fileMenu->addAction(connect_action_);
  fileMenu->addAction(load_from_file_action_);
  fileMenu->addAction(import_action_);
  fileMenu->addAction(export_action_);
  QMenu* recentMenu = new QMenu(this);
  recent_connections_ = fileMenu->addMenu(recentMenu);
  for (auto i = 0; i < max_recent_connections; ++i) {
    recent_connections_acts_[i] = new QAction(this);
    VERIFY(connect(recent_connections_acts_[i], &QAction::triggered, this, &MainWindow::openRecentConnection));
    recentMenu->addAction(recent_connections_acts_[i]);
  }

  clear_menu_ = new QAction(this);
  recentMenu->addSeparator();
  VERIFY(connect(clear_menu_, &QAction::triggered, this, &MainWindow::clearRecentConnectionsMenu));
  recentMenu->addAction(clear_menu_);

  fileMenu->addSeparator();
  fileMenu->addAction(exit_action_);
  updateRecentConnectionActions();

  preferences_action_ = new QAction(this);
  preferences_action_->setIcon(GuiFactory::GetInstance().GetPreferencesIcon());
  VERIFY(connect(preferences_action_, &QAction::triggered, this, &MainWindow::openPreferences));

  // edit menu
  QMenu* editMenu = new QMenu(this);
  edit_action_ = menuBar()->addMenu(editMenu);
  editMenu->addAction(preferences_action_);

  // tools menu
  QMenu* tools = new QMenu(this);
  tools_action_ = menuBar()->addMenu(tools);

  encode_decode_dialog_action_ = new QAction(this);
  encode_decode_dialog_action_->setIcon(GuiFactory::GetInstance().GetEncodeDecodeIcon());
  VERIFY(connect(encode_decode_dialog_action_, &QAction::triggered, this, &MainWindow::openEncodeDecodeDialog));
  tools->addAction(encode_decode_dialog_action_);

  // window menu
  QMenu* window = new QMenu(this);
  window_action_ = menuBar()->addMenu(window);
  full_screan_action_ = new QAction(this);
  full_screan_action_->setShortcut(g_full_screen_key);
  VERIFY(connect(full_screan_action_, &QAction::triggered, this, &MainWindow::enterLeaveFullScreen));
  window->addAction(full_screan_action_);

  QMenu* views = new QMenu(translations::trViews, this);
  window->addMenu(views);

  QMenu* helpMenu = new QMenu(this);
  about_action_ = new QAction(this);
  VERIFY(connect(about_action_, &QAction::triggered, this, &MainWindow::about));

  howtouse_action_ = new QAction(this);
  VERIFY(connect(howtouse_action_, &QAction::triggered, this, &MainWindow::howToUse));

  help_action_ = menuBar()->addMenu(helpMenu);

  check_update_action_ = new QAction(this);
  VERIFY(connect(check_update_action_, &QAction::triggered, this, &MainWindow::checkUpdate));

  report_bug_action_ = new QAction(this);
  VERIFY(connect(report_bug_action_, &QAction::triggered, this, &MainWindow::reportBug));

  helpMenu->addAction(check_update_action_);
  helpMenu->addSeparator();
  helpMenu->addAction(report_bug_action_);
  helpMenu->addAction(howtouse_action_);
  helpMenu->addSeparator();
  helpMenu->addAction(about_action_);

  MainWidget* mainW = new MainWidget;
  setCentralWidget(mainW);

  exp_ = new ExplorerTreeWidget(this);
  VERIFY(connect(exp_, &ExplorerTreeWidget::consoleOpened, mainW, &MainWidget::openConsole));
  VERIFY(connect(exp_, &ExplorerTreeWidget::consoleOpenedAndExecute, mainW, &MainWidget::openConsoleAndExecute));
  VERIFY(connect(exp_, &ExplorerTreeWidget::serverClosed, this, &MainWindow::closeServer, Qt::DirectConnection));
  VERIFY(connect(exp_, &ExplorerTreeWidget::clusterClosed, this, &MainWindow::closeCluster, Qt::DirectConnection));
  VERIFY(connect(exp_, &ExplorerTreeWidget::sentinelClosed, this, &MainWindow::closeSentinel, Qt::DirectConnection));
  exp_dock_ = new QDockWidget(this);
  explorer_action_ = exp_dock_->toggleViewAction();
  explorer_action_->setShortcut(explorerKeySequence);
  explorer_action_->setChecked(true);
  views->addAction(explorer_action_);

  exp_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea |
                             Qt::TopDockWidgetArea);
  exp_dock_->setWidget(exp_);
  exp_dock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  exp_dock_->setVisible(true);
  addDockWidget(Qt::LeftDockWidgetArea, exp_dock_);

  LogTabWidget* log = new LogTabWidget(this);
  VERIFY(connect(&common::qt::Logger::GetInstance(), &common::qt::Logger::printed, log, &LogTabWidget::addLogMessage));
  VERIFY(connect(&proxy::CommandLogger::GetInstance(), &proxy::CommandLogger::Printed, log, &LogTabWidget::addCommand));
  SET_LOG_WATCHER(&LogWatcherRedirect);
  log_dock_ = new QDockWidget(this);
  logs_action_ = log_dock_->toggleViewAction();
  logs_action_->setShortcut(logsKeySequence);
  logs_action_->setChecked(true);
  views->addAction(logs_action_);

  log_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea |
                             Qt::TopDockWidgetArea);
  log_dock_->setWidget(log);
  log_dock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  log_dock_->setVisible(true);
  addDockWidget(Qt::BottomDockWidgetArea, log_dock_);

  setMinimumSize(QSize(min_width, min_height));
  createStatusBar();
  retranslateUi();
}

MainWindow::~MainWindow() {
  proxy::ServersManager::GetInstance().Clear();
}

void MainWindow::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  return QMainWindow::changeEvent(ev);
}

void MainWindow::showEvent(QShowEvent* ev) {
  QMainWindow::showEvent(ev);
  static bool statistic_sent = false;
  if (!statistic_sent) {
    sendStatisticAndCheckVersion();
    statistic_sent = true;
    QTimer::singleShot(0, this, SLOT(open()));
  }
}

void MainWindow::sendStatisticAndCheckVersion() {
  bool check_updates = proxy::SettingsManager::GetInstance()->GetAutoCheckUpdates();
  if (check_updates) {
    checkUpdate();
  }

  bool send_statistic = proxy::SettingsManager::GetInstance()->GetSendStatistic();
  if (send_statistic) {
    sendStatistic();
  }
}

void MainWindow::open() {
  ConnectionsDialog dlg(this);
  int result = dlg.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  if (proxy::IConnectionSettingsBaseSPtr con = dlg.selectedConnection()) {
    createServer(con);
  } else if (proxy::IClusterSettingsBaseSPtr clus = dlg.selectedCluster()) {
    createCluster(clus);
  } else if (proxy::ISentinelSettingsBaseSPtr sent = dlg.selectedSentinel()) {
    createSentinel(sent);
  } else {
    NOTREACHED();
  }
}

void MainWindow::about() {
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::howToUse() {
  HowToUseDialog dlg(this);
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
  uint32_t exec_count = proxy::SettingsManager::GetInstance()->GetExecCount();
  StatisticSender* sender = new StatisticSender(USER_SPECIFIC_LOGIN, exec_count);
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
  } else {
    showFullScreen();
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
  proxy::ConnectionSettingsPath path(srcon);
  auto conns = proxy::SettingsManager::GetInstance()->GetConnections();
  for (auto it = conns.begin(); it != conns.end(); ++it) {
    proxy::IConnectionSettingsBaseSPtr con = *it;
    if (con && con->GetPath() == path) {
      createServer(con);
      return;
    }
  }
}

void MainWindow::loadConnection() {
  QString standardIni;
  common::ConvertFromString(proxy::SettingsManager::SettingsFilePath(), &standardIni);
  QString filepathR =
      QFileDialog::getOpenFileName(this, tr("Select settings file"), standardIni, tr("Settings files (*.ini)"));
  if (filepathR.isNull()) {
    return;
  }

  proxy::SettingsManager::GetInstance()->ReloadFromPath(common::ConvertToString(filepathR), false);
  QMessageBox::information(this, translations::trInfo, trSettingsLoadedS);
}

void MainWindow::importConnection() {
  std::string dir_path = proxy::SettingsManager::SettingsDirPath();
  QString qdir_path;
  common::ConvertFromString(dir_path, &qdir_path);
  QString filepathR = QFileDialog::getOpenFileName(this, tr("Select encrypted settings file"), qdir_path,
                                                   tr("Encrypted settings files (*.cini)"));
  if (filepathR.isNull()) {
    return;
  }

  std::string tmp = proxy::SettingsManager::SettingsFilePath() + ".tmp";

  common::file_system::ascii_string_path wp(tmp);
  common::file_system::ANSIFile writeFile(wp);
  common::ErrnoError err = writeFile.Open("wb");
  if (err) {
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  common::file_system::ascii_string_path rp(common::ConvertToString(filepathR));
  common::file_system::ANSIFile readFile(rp);
  err = readFile.Open("rb");
  if (err) {
    writeFile.Close();
    err = common::file_system::remove_file(wp.GetPath());
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  common::IEDcoder* hexEnc = common::CreateEDCoder(common::ED_HEX);
  if (!hexEnc) {
    readFile.Close();
    writeFile.Close();
    err = common::file_system::remove_file(wp.GetPath());
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
    return;
  }

  while (!readFile.IsEOF()) {
    std::string data;
    bool res = readFile.Read(&data, 1024);
    if (!res) {
      break;
    }

    if (data.empty()) {
      continue;
    }

    std::string edata;
    common::Error err = hexEnc->Decode(data, &edata);
    if (err) {
      readFile.Close();
      writeFile.Close();
      common::ErrnoError err = common::file_system::remove_file(wp.GetPath());
      if (err) {
        DNOTREACHED();
      }
      QMessageBox::critical(this, translations::trError, trImportSettingsFailed);
      return;
    } else {
      writeFile.Write(edata);
    }
  }

  readFile.Close();
  writeFile.Close();
  proxy::SettingsManager::GetInstance()->ReloadFromPath(tmp, false);
  err = common::file_system::remove_file(tmp);
  if (err) {
    DNOTREACHED();
  }
  QMessageBox::information(this, translations::trInfo, trSettingsImportedS);
}

void MainWindow::exportConnection() {
  std::string dir_path = proxy::SettingsManager::SettingsDirPath();
  QString qdir;
  common::ConvertFromString(dir_path, &qdir);
  QString filepathW = ShowSaveFileDialog(this, tr("Select file to save settings"), qdir, tr("Settings files (*.cini)"));
  if (filepathW.isEmpty()) {
    return;
  }

  common::file_system::ascii_string_path wp(common::ConvertToString(filepathW));
  common::file_system::ANSIFile writeFile(wp);
  common::ErrnoError err = writeFile.Open("wb");
  if (err) {
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  common::file_system::ascii_string_path rp(proxy::SettingsManager::SettingsFilePath());
  common::file_system::ANSIFile readFile(rp);
  err = readFile.Open("rb");
  if (err) {
    writeFile.Close();
    common::ErrnoError err = common::file_system::remove_file(wp.GetPath());
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  common::IEDcoder* hexEnc = common::CreateEDCoder(common::ED_HEX);
  if (!hexEnc) {
    readFile.Close();
    writeFile.Close();
    common::ErrnoError err = common::file_system::remove_file(wp.GetPath());
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
    return;
  }

  while (!readFile.IsEOF()) {
    std::string data;
    bool res = readFile.Read(&data, 512);
    if (!res) {
      break;
    }

    if (data.empty()) {
      continue;
    }

    std::string edata;
    common::Error er = hexEnc->Encode(data, &edata);
    if (err) {
      readFile.Close();
      writeFile.Close();
      common::ErrnoError err = common::file_system::remove_file(wp.GetPath());
      if (err) {
        DNOTREACHED();
      }
      QMessageBox::critical(this, translations::trError, trExportSettingsFailed);
      return;
    } else {
      writeFile.Write(edata);
    }
  }

  readFile.Close();
  writeFile.Close();
  QMessageBox::information(this, translations::trInfo, trSettingsExportedS);
}

void MainWindow::versionAvailible(bool succesResult, const QString& version) {
  if (!succesResult) {
    QMessageBox::information(this, translations::trCheckVersion, translations::trConnectionErrorText);
    check_update_action_->setEnabled(true);
    return;
  }

  std::string sver = common::ConvertToString(version);
  bool is_need_update = IsNeededUpdate(sver);
  if (is_need_update) {
    QMessageBox::information(
        this, translations::trCheckVersion,
        QObject::tr("<h4>A new version(%1) of " PROJECT_NAME_TITLE " is availible!</h4>"
                    "You can download it in your <a href=\"" PROJECT_DOWNLOAD_LINK "\">profile page</a>")
            .arg(version));
  }

  check_update_action_->setEnabled(is_need_update);
}

void MainWindow::statitsticSent(bool succesResult) {
  if (succesResult) {
    proxy::SettingsManager::GetInstance()->SetSendStatistic(true);
  }
}

void MainWindow::closeServer(proxy::IServerSPtr server) {
  proxy::ServersManager::GetInstance().CloseServer(server);
}

void MainWindow::closeSentinel(proxy::ISentinelSPtr sentinel) {
  proxy::ServersManager::GetInstance().CloseSentinel(sentinel);
}

void MainWindow::closeCluster(proxy::IClusterSPtr cluster) {
  proxy::ServersManager::GetInstance().CloseCluster(cluster);
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

void MainWindow::createStatusBar() {}

void MainWindow::retranslateUi() {
  connect_action_->setText(translations::trConnect + "...");
  load_from_file_action_->setText(translations::trLoadFromFile);
  import_action_->setText(translations::trImportSettings);
  export_action_->setText(translations::trExportSettings);
  exit_action_->setText(translations::trExit);
  file_action_->setText(translations::trFile);
  tools_action_->setText(translations::trTools);
  encode_decode_dialog_action_->setText(translations::trEncodeDecode);
  preferences_action_->setText(translations::trPreferences);
  check_update_action_->setText(translations::trCheckUpdate);
  edit_action_->setText(translations::trEdit);
  window_action_->setText(translations::trWindow);
  full_screan_action_->setText(translations::trFullScreen);
  report_bug_action_->setText(translations::trReportBug);
  about_action_->setText(tr("About %1...").arg(PROJECT_NAME_TITLE));
  howtouse_action_->setText(translations::trHowToUse);
  help_action_->setText(translations::trHelp);
  explorer_action_->setText(translations::trExpTree);
  logs_action_->setText(translations::trLogs);
  recent_connections_->setText(translations::trRecentConnections);
  clear_menu_->setText(translations::trClearMenu);

  exp_dock_->setWindowTitle(translations::trExpTree);
  log_dock_->setWindowTitle(translations::trLogs);
}

void MainWindow::updateRecentConnectionActions() {
  QStringList connections = proxy::SettingsManager::GetInstance()->GetRecentConnections();

  int num_recent_files = std::min(connections.size(), static_cast<int>(max_recent_connections));
  for (int i = 0; i < num_recent_files; ++i) {
    QString text = connections[i];
    recent_connections_acts_[i]->setText(text);
    recent_connections_acts_[i]->setVisible(true);
  }

  for (int j = num_recent_files; j < max_recent_connections; ++j) {
    recent_connections_acts_[j]->setVisible(false);
  }

  bool have_items = num_recent_files > 0;
  clear_menu_->setVisible(have_items);
  recent_connections_->setEnabled(have_items);
}

void MainWindow::clearRecentConnectionsMenu() {
  proxy::SettingsManager::GetInstance()->ClearRConnections();
  updateRecentConnectionActions();
}

void MainWindow::createServer(proxy::IConnectionSettingsBaseSPtr settings) {
  CHECK(settings);

  std::string path = settings->GetPath().ToString();
  QString rcon;
  common::ConvertFromString(path, &rcon);
  proxy::SettingsManager::GetInstance()->RemoveRConnection(rcon);
  proxy::IServerSPtr server = proxy::ServersManager::GetInstance().CreateServer(settings);
  exp_->addServer(server);
  proxy::SettingsManager::GetInstance()->AddRConnection(rcon);
  updateRecentConnectionActions();
  if (proxy::SettingsManager::GetInstance()->GetAutoConnectDB()) {
    proxy::events_info::ConnectInfoRequest req(this);
    server->Connect(req);
  }

  if (!proxy::SettingsManager::GetInstance()->AutoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    mwidg->openConsole(server, QString());
  }
}

void MainWindow::createSentinel(proxy::ISentinelSettingsBaseSPtr settings) {
  CHECK(settings);

  proxy::ISentinelSPtr sent = proxy::ServersManager::GetInstance().CreateSentinel(settings);
  if (!sent) {
    return;
  }

  auto sentinels = sent->GetSentinels();
  if (sentinels.empty()) {
    return;
  }

  exp_->addSentinel(sent);
  if (!proxy::SettingsManager::GetInstance()->AutoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    for (size_t i = 0; i < sentinels.size(); ++i) {
      proxy::IServerSPtr serv = sentinels[i].sentinel;
      mwidg->openConsole(serv, QString());
    }
  }
}

void MainWindow::createCluster(proxy::IClusterSettingsBaseSPtr settings) {
  CHECK(settings);

  proxy::IClusterSPtr cl = proxy::ServersManager::GetInstance().CreateCluster(settings);
  if (!cl) {
    return;
  }

  proxy::IServerSPtr root = cl->GetRoot();
  if (!root) {
    return;
  }

  exp_->addCluster(cl);
  if (!proxy::SettingsManager::GetInstance()->AutoOpenConsole()) {
    return;
  }

  MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
  if (mwidg) {
    mwidg->openConsole(root, QString());
  }
}

}  // namespace gui
}  // namespace fastonosql
