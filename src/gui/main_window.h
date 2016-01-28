/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QMainWindow>

#include "core/connection_settings.h"

class QAction;
class QDockWidget;

#ifdef OS_ANDROID
class QGestureEvent;
class QSwipeGesture;
class QTapAndHoldGesture;
#endif

namespace fastonosql {
class MainWidget;
class ExplorerTreeView;

class MainWindow
  : public QMainWindow {
  Q_OBJECT
public:
  enum
  {
    min_height = 480,
    min_width = 640,
    MaxRecentConnections = 5
  };

  MainWindow();
  ~MainWindow();

protected:
  virtual void changeEvent(QEvent* ev);
  virtual void showEvent(QShowEvent* ev);

private Q_SLOTS:
  void open();
  void about();
  void openPreferences();
  void checkUpdate();
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

protected:
#ifdef OS_ANDROID
  virtual bool event(QEvent *event);
  bool gestureEvent(QGestureEvent *event);
  void swipeTriggered(QSwipeGesture* swipeEvent);
  void tapAndHoldTriggered(QTapAndHoldGesture* tapEvent);
#endif

private:
  void createToolBar();
  void createStatusBar();
  void retranslateUi();
  void updateRecentConnectionActions();
  void clearRecentConnectionsMenu();
  void createServer(IConnectionSettingsBaseSPtr settings);
  void createCluster(IClusterSettingsBaseSPtr settings);

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
  QAction* recentConnectionsActs_[MaxRecentConnections];
#ifdef BUILD_WITH_SOCIAL_BUTTONS
  QAction* homePageAction_;
  QAction* facebookAction_;
  QAction* twitterAction_;
  QAction* githubAction_;
#endif
  ExplorerTreeView* exp_;
  QDockWidget* expDock_;
  QDockWidget* logDock_;
  bool isCheckedInSession_;
};

class UpdateChecker
  : public QObject {
  Q_OBJECT
public:
  explicit UpdateChecker(QObject* parent = 0);

Q_SIGNALS:
  void versionAvailibled(bool succesResult, const QString& version);

public Q_SLOTS:
  void routine();
};

}
