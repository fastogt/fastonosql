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

#pragma once

#include <QTreeView>

#include "proxy/events/events_info.h"
#include "proxy/proxy_fwd.h"  // for IServerSPtr, IClusterSPtr, etc

class QAction;  // lines 23-23
class QPoint;
class QSortFilterProxyModel;

namespace fastonosql {
namespace gui {
class ExplorerTreeModel;

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
  void createDb();
  void removeDB();
  void openInfoServerDialog();
  void openPropertyServerDialog();
  void openHistoryServerDialog();
  void clearHistory();
  void closeServerConnection();
  void closeClusterConnection();
  void closeSentinelConnection();

  void importServer();
  void exportServer();

  void loadContentDb();
  void removeAllKeys();
  void removeBranch();
  void setDefaultDb();
  void removeDb();
  void createKey();
  void editKey();
  void viewKeys();
  void viewPubSub();

  void loadValue();
  void renKey();
  void deleteKey();
  void watchKey();
  void setTTL();
  void removeTTL();

  void startLoadDatabases(const proxy::events_info::LoadDatabasesInfoRequest& req);
  void finishLoadDatabases(const proxy::events_info::LoadDatabasesInfoResponce& res);

  void startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponce& res);

  void startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res);

  void createDatabase(core::IDataBaseInfoSPtr db);
  void removeDatabase(core::IDataBaseInfoSPtr db);

  void flushDB(core::IDataBaseInfoSPtr db);
  void currentDataBaseChange(core::IDataBaseInfoSPtr db);
  void removeKey(core::IDataBaseInfoSPtr db, core::NKey key);
  void addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void renameKey(core::IDataBaseInfoSPtr db, core::NKey key, core::key_t new_name);
  void loadKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void changeTTLKey(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);

 protected:
  virtual void changeEvent(QEvent* ev) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* ev) override;

 private:
  void syncWithServer(proxy::IServer* server);
  void unsyncWithServer(proxy::IServer* server);

  void retranslateUi();
  QModelIndexList selectedEqualTypeIndexes() const;

  ExplorerTreeModel* source_model_;
  QSortFilterProxyModel* proxy_model_;
};

}  // namespace gui
}  // namespace fastonosql
