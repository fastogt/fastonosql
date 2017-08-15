/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/qt/gui/base/tree_model.h>  // for TreeModel

#include "proxy/database/idatabase.h"
#include "proxy/proxy_fwd.h"
#include "proxy/types.h"

class QModelIndex;
class QObject;

namespace fastonosql {
namespace proxy {
class IServer;
}
}  // namespace fastonosql

namespace fastonosql {
namespace gui {
class ExplorerClusterItem;
class ExplorerSentinelItem;
class ExplorerServerItem;
class ExplorerDatabaseItem;
class ExplorerKeyItem;
class ExplorerNSItem;
class IExplorerTreeItem;
}  // namespace gui
}  // namespace fastonosql

namespace fastonosql {
namespace gui {
class ExplorerTreeModel : public common::qt::gui::TreeModel {
  Q_OBJECT
 public:
  explicit ExplorerTreeModel(QObject* parent = 0);

  virtual QVariant data(const QModelIndex& index, int role) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual int columnCount(const QModelIndex& parent) const override;

  void addCluster(proxy::IClusterSPtr cluster);
  void removeCluster(proxy::IClusterSPtr cluster);

  void addServer(proxy::IServerSPtr server);
  void removeServer(proxy::IServerSPtr server);

  void addSentinel(proxy::ISentinelSPtr sentinel);
  void removeSentinel(proxy::ISentinelSPtr sentinel);

  void addDatabase(proxy::IServer* server, core::IDataBaseInfoSPtr db);
  void removeDatabase(proxy::IServer* server, core::IDataBaseInfoSPtr db);
  void setDefaultDb(proxy::IServer* server, core::IDataBaseInfoSPtr db);
  void updateDb(proxy::IServer* server, core::IDataBaseInfoSPtr db);

  void addKey(proxy::IServer* server,
              core::IDataBaseInfoSPtr db,
              const core::NDbKValue& dbv,
              const std::string& ns_separator);
  void removeKey(proxy::IServer* server, core::IDataBaseInfoSPtr db, const core::NKey& key);
  void updateKey(proxy::IServer* server,
                 core::IDataBaseInfoSPtr db,
                 const core::NKey& old_key,
                 const core::NKey& new_key);
  void updateValue(proxy::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue& dbv);
  void removeAllKeys(proxy::IServer* server, core::IDataBaseInfoSPtr db);

 private:
  ExplorerClusterItem* findClusterItem(proxy::IClusterSPtr cl);
  ExplorerSentinelItem* findSentinelItem(proxy::ISentinelSPtr sentinel);
  ExplorerServerItem* findServerItem(proxy::IServer* server) const;
  ExplorerDatabaseItem* findDatabaseItem(ExplorerServerItem* server, core::IDataBaseInfoSPtr db) const;
  ExplorerKeyItem* findKeyItem(IExplorerTreeItem* db_or_ns, const core::NKey& key) const;
  ExplorerNSItem* findNSItem(IExplorerTreeItem* db_or_ns, const QString& name) const;
  ExplorerNSItem* findOrCreateNSItem(IExplorerTreeItem* db_or_ns, const proxy::KeyInfo& kinf);
};
}  // namespace gui
}  // namespace fastonosql
