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

#include <QString>

#include <common/qt/gui/base/tree_item.h>  // for TreeItem

#include "proxy/proxy_fwd.h"  // for IServerSPtr, IClusterSPtr, etc
#include "core/db_key.h"    // for NDbKValue, etc
#include "core/database/idatabase_info.h"

namespace fastonosql {
namespace core {
class IServer;
}
}

namespace fastonosql {
namespace gui {
class IExplorerTreeItem : public common::qt::gui::TreeItem {
 public:
  enum eColumn { eName = 0, eCountColumns };

  enum eType { eCluster, eSentinel, eServer, eDatabase, eNamespace, eKey };

  explicit IExplorerTreeItem(TreeItem* parent);

  virtual QString name() const = 0;
  virtual eType type() const = 0;
};

class ExplorerServerItem : public IExplorerTreeItem {
 public:
  ExplorerServerItem(proxy::IServerSPtr server, TreeItem* parent);

  virtual QString name() const override;
  proxy::IServerSPtr server() const;
  virtual eType type() const override;

  void loadDatabases();

 private:
  const proxy::IServerSPtr server_;
};

class ExplorerSentinelItem : public IExplorerTreeItem {
 public:
  ExplorerSentinelItem(proxy::ISentinelSPtr sentinel, TreeItem* parent);

  virtual QString name() const override;
  virtual eType type() const override;

  proxy::ISentinelSPtr sentinel() const;

 private:
  const proxy::ISentinelSPtr sentinel_;
};

class ExplorerClusterItem : public IExplorerTreeItem {
 public:
  ExplorerClusterItem(proxy::IClusterSPtr cluster, TreeItem* parent);

  virtual QString name() const override;
  virtual eType type() const override;

  proxy::IClusterSPtr cluster() const;

 private:
  const proxy::IClusterSPtr cluster_;
};

class ExplorerDatabaseItem : public IExplorerTreeItem {
 public:
  ExplorerDatabaseItem(proxy::IDatabaseSPtr db, ExplorerServerItem* parent);

  virtual QString name() const override;
  virtual eType type() const override;
  bool isDefault() const;
  size_t totalKeysCount() const;
  size_t loadedKeysCount() const;

  proxy::IServerSPtr server() const;
  proxy::IDatabaseSPtr db() const;

  void loadContent(const std::string& pattern, uint32_t countKeys);
  void setDefault();

  core::IDataBaseInfoSPtr info() const;

  void renameKey(const core::NKey& key, const QString& newName);
  void removeKey(const core::NKey& key);
  void loadValue(const core::NDbKValue& key);
  void watchKey(const core::NDbKValue& key, int interval);
  void createKey(const core::NDbKValue& key);
  void editKey(const core::NDbKValue& key, const core::NValue& value);
  void setTTL(const core::NKey& key, core::ttl_t ttl);

  void removeAllKeys();

 private:
  const proxy::IDatabaseSPtr db_;
};

class ExplorerNSItem : public IExplorerTreeItem {
 public:
  ExplorerNSItem(const QString& name, IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  virtual QString name() const override;
  proxy::IServerSPtr server() const;
  virtual eType type() const override;
  size_t keyCount() const;

  void removeBranch();

 private:
  QString name_;
};

class ExplorerKeyItem : public IExplorerTreeItem {
 public:
  ExplorerKeyItem(const core::NDbKValue& dbv, IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  core::NDbKValue dbv() const;
  void setDbv(const core::NDbKValue& key);

  core::NKey key() const;
  void setKey(const core::NKey& key);

  virtual QString name() const override;
  proxy::IServerSPtr server() const;
  virtual eType type() const override;

  void renameKey(const QString& newName);
  void editKey(const core::NValue& value);
  void removeFromDb();
  void watchKey(int interval);
  void loadValueFromDb();
  void setTTL(core::ttl_t ttl);

 private:
  core::NDbKValue dbv_;
};
}  // namespace gui
}  // namespace fastonosql
