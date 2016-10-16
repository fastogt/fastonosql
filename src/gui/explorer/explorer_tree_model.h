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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t
#include <string>    // for string

#include <common/qt/gui/base/tree_model.h>  // for TreeModel

#include "core/core_fwd.h"
#include "core/database/idatabase.h"

class QModelIndex;
class QObject;

namespace fastonosql {
namespace core {
class IServer;
}
}

namespace fastonosql {
namespace gui {

class ExplorerClusterItem;
class ExplorerSentinelItem;
class ExplorerServerItem;
class ExplorerDatabaseItem;
class ExplorerKeyItem;
class ExplorerNSItem;
class IExplorerTreeItem;

class ExplorerTreeModel : public common::qt::gui::TreeModel {
  Q_OBJECT
 public:
  explicit ExplorerTreeModel(QObject* parent = 0);

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual int columnCount(const QModelIndex& parent) const;

  void addCluster(core::IClusterSPtr cluster);
  void removeCluster(core::IClusterSPtr cluster);

  void addServer(core::IServerSPtr server);
  void removeServer(core::IServerSPtr server);

  void addSentinel(core::ISentinelSPtr sentinel);
  void removeSentinel(core::ISentinelSPtr sentinel);

  void addDatabase(core::IServer* server, core::IDataBaseInfoSPtr db);
  void removeDatabase(core::IServer* server, core::IDataBaseInfoSPtr db);
  void setDefaultDb(core::IServer* server, core::IDataBaseInfoSPtr db);
  void updateDb(core::IServer* server, core::IDataBaseInfoSPtr db);

  void addKey(core::IServer* server,
              core::IDataBaseInfoSPtr db,
              const core::NDbKValue& dbv,
              const std::string& ns_separator);
  void removeKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NKey& key);
  void updateKey(core::IServer* server,
                 core::IDataBaseInfoSPtr db,
                 const core::NKey& old_key,
                 const core::NKey& new_key);
  void updateValue(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue& dbv);
  void removeAllKeys(core::IServer* server, core::IDataBaseInfoSPtr db);

 private:
  ExplorerClusterItem* findClusterItem(core::IClusterSPtr cl);
  ExplorerSentinelItem* findSentinelItem(core::ISentinelSPtr sentinel);
  ExplorerServerItem* findServerItem(core::IServer* server) const;
  ExplorerDatabaseItem* findDatabaseItem(ExplorerServerItem* server,
                                         core::IDataBaseInfoSPtr db) const;
  ExplorerKeyItem* findKeyItem(IExplorerTreeItem* db_or_ns, const core::NKey& key) const;
  ExplorerNSItem* findNSItem(IExplorerTreeItem* db_or_ns, const QString& name) const;
  ExplorerNSItem* findOrCreateNSItem(IExplorerTreeItem* db_or_ns, const core::KeyInfo& kinf);
};

}  // namespace gui
}  // namespace fastonosql
