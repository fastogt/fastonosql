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

#include "proxy/proxy_fwd.h"  // for IServerSPtr, IClusterSPtr, etc
#include "proxy/events/events_info.h"

class QAction;  // lines 23-23
class QEvent;
class QMouseEvent;
class QPoint;
class QWidget;
class QSortFilterProxyModel;

namespace fastonosql {
namespace gui {
class ExplorerTreeModel;
}
}

namespace fastonosql {
namespace gui {

class ExplorerTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit ExplorerTreeView(QWidget* parent);

 Q_SIGNALS:
  void consoleOpened(proxy::IServerSPtr server, const QString& text);
  void consoleOpenedAndExecute(proxy::IServerSPtr server, const QString& text);
  void serverClosed(proxy::IServerSPtr server);
  void sentinelClosed(proxy::ISentinelSPtr sentinel);
  void clusterClosed(proxy::IClusterSPtr cluster);

 public Q_SLOTS:
  void addServer(proxy::IServerSPtr server);
  void removeServer(proxy::IServerSPtr server);

  void addSentinel(proxy::ISentinelSPtr sentinel);
  void removeSentinel(proxy::ISentinelSPtr sentinel);

  void addCluster(proxy::IClusterSPtr cluster);
  void removeCluster(proxy::IClusterSPtr cluster);

  void changeTextFilter(const QString& text);

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
  void closeSentinelConnection();

  void backupServer();
  void importServer();
  void shutdownServer();

  void loadContentDb();
  void removeAllKeys();
  void removeBranch();
  void setDefaultDb();
  void createKey();
  void editKey();
  void viewKeys();
  void viewPubSub();

  void loadValue();
  void renKey();
  void deleteKey();
  void watchKey();
  void setTTL();

  void startLoadDatabases(const proxy::events_info::LoadDatabasesInfoRequest& req);
  void finishLoadDatabases(const proxy::events_info::LoadDatabasesInfoResponce& res);

  void startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponce& res);

  void startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res);

  void flushDB(core::IDataBaseInfoSPtr db);
  void currentDataBaseChange(core::IDataBaseInfoSPtr db);
  void removeKey(core::IDataBaseInfoSPtr db, core::NKey key);
  void addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void renameKey(core::IDataBaseInfoSPtr db, core::NKey key, std::string new_name);
  void loadKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void changeTTLKey(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);

 protected:
  virtual void changeEvent(QEvent* ev) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* ev) override;

 private:
  void syncWithServer(proxy::IServer* server);
  void unsyncWithServer(proxy::IServer* server);

  void retranslateUi();
  QModelIndex selectedIndex() const;

  QAction* connectAction_;
  QAction* openConsoleAction_;
  QAction* loadDatabaseAction_;
  QAction* loadContentAction_;
  QAction* removeAllKeysAction_;
  QAction* removeBranchAction_;
  QAction* setDefaultDbAction_;
  QAction* createKeyAction_;
  QAction* editKeyAction_;
  QAction* viewKeysAction_;
  QAction* pubSubAction_;
  QAction* getValueAction_;
  QAction* renameKeyAction_;
  QAction* deleteKeyAction_;
  QAction* watchKeyAction_;
  QAction* infoServerAction_;
  QAction* propertyServerAction_;
  QAction* setServerPassword_;
  QAction* setMaxClientConnection_;
  QAction* historyServerAction_;
  QAction* clearHistoryServerAction_;
  QAction* closeServerAction_;
  QAction* closeClusterAction_;
  QAction* closeSentinelAction_;
  QAction* importAction_;
  QAction* backupAction_;
  QAction* shutdownAction_;
  ExplorerTreeModel* source_model_;
  QSortFilterProxyModel* proxy_model_;
};

}  // namespace gui
}  // namespace fastonosql
