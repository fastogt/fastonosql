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

#include <stddef.h>                     // for size_t
#include <stdint.h>                     // for uint32_t
#include <string>                       // for string

#include "core/core_fwd.h"              // for IServerSPtr, IClusterSPtr, etc
#include "core/db_key.h"                // for NDbKValue, etc
#include "core/types.h"                 // for IDataBaseInfoSPtr

#include "fasto/qt/gui/base/tree_item.h"  // for TreeItem
#include "fasto/qt/gui/base/tree_model.h"  // for TreeModel

class QModelIndex;
class QObject;

namespace fastonosql { namespace core { class IServer; } }

namespace fastonosql {
namespace gui {

class IExplorerTreeItem
  : public fasto::qt::gui::TreeItem {
 public:
  enum eColumn {
    eName = 0,
    eCountColumns
  };

  enum eType {
    eCluster,
    eSentinel,
    eServer,
    eDatabase,
    eNamespace,
    eKey
  };

  explicit IExplorerTreeItem(TreeItem* parent);

  virtual QString name() const = 0;
  virtual eType type() const = 0;
};

class ExplorerServerItem
  : public IExplorerTreeItem {
 public:
  ExplorerServerItem(core::IServerSPtr server, TreeItem* parent);

  virtual QString name() const;
  core::IServerSPtr server() const;
  virtual eType type() const;

  void loadDatabases();

 private:
  const core::IServerSPtr server_;
};

class ExplorerSentinelItem
  : public IExplorerTreeItem {
 public:
  ExplorerSentinelItem(core::ISentinelSPtr sentinel, TreeItem* parent);

  virtual QString name() const;
  virtual eType type() const;

  core::ISentinelSPtr sentinel() const;

 private:
  const core::ISentinelSPtr sentinel_;
};

class ExplorerClusterItem
  : public IExplorerTreeItem {
 public:
  ExplorerClusterItem(core::IClusterSPtr cluster, TreeItem* parent);

  virtual QString name() const;
  virtual eType type() const;

  core::IClusterSPtr cluster() const;

 private:
  const core::IClusterSPtr cluster_;
};

class ExplorerDatabaseItem
  : public IExplorerTreeItem {
 public:
  ExplorerDatabaseItem(core::IDatabaseSPtr db, ExplorerServerItem* parent);

  virtual QString name() const;
  virtual eType type() const;
  bool isDefault() const;
  size_t totalKeysCount() const;
  size_t loadedKeysCount() const;

  core::IServerSPtr server() const;
  core::IDatabaseSPtr db() const;

  void loadContent(const std::string& pattern, uint32_t countKeys);
  void setDefault();

  core::IDataBaseInfoSPtr info() const;

  void removeKey(const core::NDbKValue& key);
  void loadValue(const core::NDbKValue& key);
  void createKey(const core::NDbKValue& key);
  void setTTL(const core::NDbKValue& key, core::ttl_t ttl);

  void removeAllKeys();

 private:
  const core::IDatabaseSPtr db_;
};

class ExplorerNSItem
  : public IExplorerTreeItem {
 public:
  ExplorerNSItem(const QString& name, IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  virtual QString name() const;
  core::IServerSPtr server() const;
  virtual eType type() const;
  size_t keyCount() const;

  void removeBranch();

 private:
  QString name_;
};

class ExplorerKeyItem
  : public IExplorerTreeItem {
 public:
  ExplorerKeyItem(const core::NDbKValue& key, IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  core::NDbKValue key() const;
  void setKey(const core::NDbKValue& key);

  virtual QString name() const;
  core::IServerSPtr server() const;
  virtual eType type() const;

  void removeFromDb();
  void loadValueFromDb();
  void setTTL(core::ttl_t ttl);

 private:
  core::NDbKValue key_;
};

class ExplorerTreeModel
  : public fasto::qt::gui::TreeModel {
  Q_OBJECT
 public:
  explicit ExplorerTreeModel(QObject* parent = 0);

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual int columnCount(const QModelIndex &parent) const;

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

  void addKey(core::IServer* server, core::IDataBaseInfoSPtr db,
              const core::NDbKValue &dbv, const std::string& ns_separator);
  void removeKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue &key);
  void updateKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue &key);
  void removeAllKeys(core::IServer* server, core::IDataBaseInfoSPtr db);

 private:
  ExplorerClusterItem* findClusterItem(core::IClusterSPtr cl);
  ExplorerSentinelItem* findSentinelItem(core::ISentinelSPtr sentinel);
  ExplorerServerItem* findServerItem(core::IServer* server) const;
  ExplorerDatabaseItem* findDatabaseItem(ExplorerServerItem* server, core::IDataBaseInfoSPtr db) const;
  ExplorerKeyItem* findKeyItem(IExplorerTreeItem* db_or_ns, const core::NDbKValue& key) const;
  ExplorerNSItem* findNSItem(IExplorerTreeItem* db_or_ns, const QString& name) const;
  ExplorerNSItem* findOrCreateNSItem(IExplorerTreeItem* db_or_ns, const core::KeyInfo& kinf);
};

}  // namespace gui
}  // namespace fastonosql
