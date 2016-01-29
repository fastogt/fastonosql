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

#include <QTreeView>

class QAction;

#include "core/iserver.h"

namespace fastonosql {
class ExplorerTreeView
      : public QTreeView
{
  Q_OBJECT

 public:
  explicit ExplorerTreeView(QWidget* parent);

 Q_SIGNALS:
  void openedConsole(IServerSPtr server, const QString& text);
  void closeServer(IServerSPtr server);
  void closeCluster(IClusterSPtr cluster);

 public Q_SLOTS:
  void addServer(IServerSPtr server);
  void removeServer(IServerSPtr server);

  void addCluster(IClusterSPtr cluster);
  void removeCluster(IClusterSPtr cluster);

 private Q_SLOTS:
  void showContextMenu(const QPoint& point);
  void connectDisconnectToServer();
  void openConsole();
  void loadDatabases();
  void openInfoServerDialog();
  void openPropertyServerDialog();
  void openSetPasswordServerDialog();
  void openMaxClientSetDialog();
  void openHistoryServerDialog();
  void clearHistory();
  void closeServerConnection();
  void closeClusterConnection();

  void backupServer();
  void importServer();
  void shutdownServer();

  void loadContentDb();
  void setDefaultDb();
  void createKey();
  void viewKeys();
  void getValue();
  void deleteKey();

  void startLoadDatabases(const EventsInfo::LoadDatabasesInfoRequest& req);
  void finishLoadDatabases(const EventsInfo::LoadDatabasesInfoResponce& res);

  void startSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req);
  void finishSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res);

  void startLoadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res);

  void startExecuteCommand(const EventsInfo::CommandRequest& req);
  void finishExecuteCommand(const EventsInfo::CommandResponce& res);

 protected:
  virtual void changeEvent(QEvent* );
  virtual void mouseDoubleClickEvent(QMouseEvent* );

 private:
  void syncWithServer(IServer* server);
  void unsyncWithServer(IServer* server);

  void retranslateUi();
  QModelIndex selectedIndex() const;

  QAction* connectAction_;
  QAction* openConsoleAction_;
  QAction* loadDatabaseAction_;
  QAction* loadContentAction_;
  QAction* setDefaultDbAction_;
  QAction* createKeyAction_;
  QAction* viewKeysAction_;
  QAction* getValueAction_;
  QAction* deleteKeyAction_;
  QAction* infoServerAction_;
  QAction* propertyServerAction_;
  QAction* setServerPassword_;
  QAction* setMaxClientConnection_;
  QAction* historyServerAction_;
  QAction* clearHistoryServerAction_;
  QAction* closeServerAction_;
  QAction* closeClusterAction_;
  QAction* importAction_;
  QAction* backupAction_;
  QAction* shutdownAction_;
};

}
