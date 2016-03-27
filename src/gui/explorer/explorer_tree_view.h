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

#include <QTreeView>

class QAction;

#include "core/core_fwd.h"
#include "core/events/events_info.h"

namespace fastonosql {
namespace gui {

class ExplorerTreeView
  : public QTreeView {
 Q_OBJECT
 public:
  explicit ExplorerTreeView(QWidget* parent);

 Q_SIGNALS:
  void openedConsole(core::IServerSPtr server, const QString& text);
  void closeServer(core::IServerSPtr server);
  void closeCluster(core::IClusterSPtr cluster);

 public Q_SLOTS:
  void addServer(core::IServerSPtr server);
  void removeServer(core::IServerSPtr server);

  void addCluster(core::IClusterSPtr cluster);
  void removeCluster(core::IClusterSPtr cluster);

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
  void removeAllKeys();
  void setDefaultDb();
  void createKey();
  void viewKeys();
  void getValue();
  void deleteKey();

  void startLoadDatabases(const core::events_info::LoadDatabasesInfoRequest& req);
  void finishLoadDatabases(const core::events_info::LoadDatabasesInfoResponce& res);

  void startSetDefaultDatabase(const core::events_info::SetDefaultDatabaseRequest& req);
  void finishSetDefaultDatabase(const core::events_info::SetDefaultDatabaseResponce& res);

  void startLoadDatabaseContent(const core::events_info::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const core::events_info::LoadDatabaseContentResponce& res);

  void startClearDatabase(const core::events_info::ClearDatabaseRequest& req);
  void finishClearDatabase(const core::events_info::ClearDatabaseResponce& res);

  void startExecuteCommand(const core::events_info::CommandRequest& req);
  void finishExecuteCommand(const core::events_info::CommandResponce& res);

 protected:
  virtual void changeEvent(QEvent* ev);
  virtual void mouseDoubleClickEvent(QMouseEvent* ev);

 private:
  void syncWithServer(core::IServer* server);
  void unsyncWithServer(core::IServer* server);

  void retranslateUi();
  QModelIndex selectedIndex() const;

  QAction* connectAction_;
  QAction* openConsoleAction_;
  QAction* loadDatabaseAction_;
  QAction* loadContentAction_;
  QAction* removeAllKeysAction_;
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

}  // namespace gui
}  // namespace fastonosql
