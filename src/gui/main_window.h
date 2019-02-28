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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QMainWindow>

#include "proxy/proxy_fwd.h"

#include "proxy/connection_settings/iconnection_settings.h"

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#endif

class QAction;
class QDockWidget;
#if defined(OS_ANDROID)
class QGestureEvent;
class QSwipeGesture;
class QTapAndHoldGesture;
#endif

namespace fastonosql {
namespace gui {

class ExplorerTreeWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  enum {
    min_width = 1024,
    min_height = 768,
    preferred_width = 1280,
    preferred_height = 800,
    max_recent_connections = 10
  };

  MainWindow();
  ~MainWindow() override;

 protected:
  void changeEvent(QEvent* ev) override;
  void showEvent(QShowEvent* ev) override;

 private Q_SLOTS:
  void open();
  void about();
  void howToUse();
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

  void versionAvailible(common::Error err, unsigned version);
  void statitsticSent(common::Error err);

  void closeServer(proxy::IServerSPtr server);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  void closeSentinel(proxy::ISentinelSPtr sentinel);
  void closeCluster(proxy::IClusterSPtr cluster);
#endif

 protected:
#if defined(OS_ANDROID)
  virtual bool event(QEvent* event);
  bool gestureEvent(QGestureEvent* event);
  void swipeTriggered(QSwipeGesture* swipeEvent);
  void tapAndHoldTriggered(QTapAndHoldGesture* tapEvent);
#endif

 private:
  void sendStatisticAndCheckVersion();

  void createStatusBar();
  void retranslateUi();
  void updateRecentConnectionActions();
  void clearRecentConnectionsMenu();
  void createServer(proxy::IConnectionSettingsBaseSPtr settings);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  void createSentinel(proxy::ISentinelSettingsBaseSPtr settings);
  void createCluster(proxy::IClusterSettingsBaseSPtr settings);
#endif

  QAction* connect_action_;
  QAction* load_from_file_action_;
  QAction* import_action_;
  QAction* export_action_;
  QAction* exit_action_;
  QAction* preferences_action_;
  QAction* full_screan_action_;
  QAction* window_action_;
  QAction* about_action_;
  QAction* howtouse_action_;
  QAction* report_bug_action_;
  QAction* file_action_;
  QAction* edit_action_;
  QAction* check_update_action_;
  QAction* tools_action_;
  QAction* encode_decode_dialog_action_;
  QAction* help_action_;
  QAction* explorer_action_;
  QAction* logs_action_;
  QAction* recent_connections_;
  QAction* clear_menu_;
  QAction* recent_connections_acts_[max_recent_connections];

  ExplorerTreeWidget* exp_;
  QDockWidget* exp_dock_;
  QDockWidget* log_dock_;
};

}  // namespace gui
}  // namespace fastonosql
