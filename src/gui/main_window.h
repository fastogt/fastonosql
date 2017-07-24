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

#pragma once

#include <QMainWindow>

#include "proxy/proxy_fwd.h"

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/connection_settings/isentinel_connection_settings.h"

class QAction;      // lines 27-27
class QDockWidget;  // lines 28-28
class QEvent;       // lines 29-29
class QShowEvent;   // lines 30-30
#ifdef OS_ANDROID
class QGestureEvent;
class QSwipeGesture;
class QTapAndHoldGesture;
#endif

namespace fastonosql {
namespace gui {
class ExplorerTreeWidget;
}
}  // namespace fastonosql

namespace fastonosql {
namespace gui {

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480, max_recent_connections = 10 };

  MainWindow();
  ~MainWindow();

 protected:
  virtual void changeEvent(QEvent* ev) override;
  virtual void showEvent(QShowEvent* ev) override;

 private Q_SLOTS:
  void open();
  void about();
  void openPreferences();
  void checkUpdate();
  void sendStatistic();
  void reportBug();
  void enterLeaveFullScreen();
  void openEncodeDecodeDialog();
  void openRecentConnection();

  void loadConnection();
  void importConnection();
  void exportConnection();

  void openHomePageLink();
  void openFacebookLink();
  void openTwitterLink();
  void openGithubLink();

  void versionAvailible(bool succesResult, const QString& version);
  void statitsticSent(bool succesResult);

  void closeServer(proxy::IServerSPtr server);
  void closeSentinel(proxy::ISentinelSPtr sentinel);
  void closeCluster(proxy::IClusterSPtr cluster);

 protected:
#ifdef OS_ANDROID
  virtual bool event(QEvent* event);
  bool gestureEvent(QGestureEvent* event);
  void swipeTriggered(QSwipeGesture* swipeEvent);
  void tapAndHoldTriggered(QTapAndHoldGesture* tapEvent);
#endif

 private:
  void createToolBar();
  void createStatusBar();
  void retranslateUi();
  void updateRecentConnectionActions();
  void clearRecentConnectionsMenu();
  void createServer(proxy::IConnectionSettingsBaseSPtr settings);
  void createSentinel(proxy::ISentinelSettingsBaseSPtr settings);
  void createCluster(proxy::IClusterSettingsBaseSPtr settings);

  QAction* openAction_;
  QAction* loadFromFileAction_;
  QAction* importAction_;
  QAction* exportAction_;
  QAction* exitAction_;
  QAction* preferencesAction_;
  QAction* fullScreanAction_;
  QAction* windowAction_;
  QAction* aboutAction_;
  QAction* reportBugAction_;
  QAction* fileAction_;
  QAction* editAction_;
  QAction* checkUpdateAction_;
  QAction* toolsAction_;
  QAction* encodeDecodeDialogAction_;
  QAction* helpAction_;
  QAction* explorerAction_;
  QAction* logsAction_;
  QAction* recentConnections_;
  QAction* clearMenu_;
  QAction* recentConnectionsActs_[max_recent_connections];

  QAction* homePageAction_;
  QAction* facebookAction_;
  QAction* twitterAction_;
  QAction* githubAction_;

  ExplorerTreeWidget* exp_;
  QDockWidget* expDock_;
  QDockWidget* logDock_;
  bool isCheckedInSession_;
};

}  // namespace gui
}  // namespace fastonosql
