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

#include <QString>

#include <common/qt/gui/base/tree_item.h>  // for TreeItem

#include <fastonosql/core/database/idatabase_info.h>

#include "proxy/proxy_fwd.h"  // for IServerSPtr, IClusterSPtr, etc
#include "proxy/types.h"

namespace fastonosql {
namespace gui {

class IExplorerTreeItem : public common::qt::gui::TreeItem {
 public:
  typedef core::readable_string_t string_t;
  enum eColumn : uint8_t { eName = 0, eCountColumns };

  enum eType : uint8_t {
    eServer = 0,
    eDatabase,
    eNamespace,
    eKey,
#if defined(PRO_VERSION)
    eCluster,
    eSentinel
#endif
  };

  IExplorerTreeItem(TreeItem* parent, eType type);

  QString name() const;
  virtual string_t basicStringName() const = 0;
  eType type() const;

 private:
  const eType type_;
};

class ExplorerServerItem : public IExplorerTreeItem {
 public:
  ExplorerServerItem(proxy::IServerSPtr server, TreeItem* parent);

  virtual string_t basicStringName() const override;
  proxy::IServerSPtr server() const;

  void loadDatabases();
  void createDatabase(const QString& name);
  void removeDatabase(const QString& name);

 private:
  const proxy::IServerSPtr server_;
};

#if defined(PRO_VERSION)
class ExplorerSentinelItem : public IExplorerTreeItem {
 public:
  ExplorerSentinelItem(proxy::ISentinelSPtr sentinel, TreeItem* parent);

  virtual string_t basicStringName() const override;

  proxy::ISentinelSPtr sentinel() const;

 private:
  const proxy::ISentinelSPtr sentinel_;
};

class ExplorerClusterItem : public IExplorerTreeItem {
 public:
  ExplorerClusterItem(proxy::IClusterSPtr cluster, TreeItem* parent);

  virtual string_t basicStringName() const override;

  proxy::IClusterSPtr cluster() const;

 private:
  const proxy::IClusterSPtr cluster_;
};
#endif

class ExplorerDatabaseItem : public IExplorerTreeItem {
 public:
  ExplorerDatabaseItem(proxy::IDatabaseSPtr db, ExplorerServerItem* parent);

  virtual string_t basicStringName() const override;
  bool isDefault() const;
  size_t totalKeysCount() const;
  size_t loadedKeysCount() const;

  proxy::IServerSPtr server() const;
  proxy::IDatabaseSPtr db() const;

  void loadContent(const core::command_buffer_t& pattern, core::keys_limit_t keys_count);
  void setDefault();
  void removeDb();

  void renameKey(const core::NKey& key, const QString& new_name);
  void removeKey(const core::NKey& key);
  void loadValue(const core::NDbKValue& key);
  void watchKey(const core::NDbKValue& key, int interval);
  void createKey(const core::NDbKValue& key);
  void editValue(const core::NDbKValue& key, const core::NValue& value);
  void setTTL(const core::NKey& key, core::ttl_t ttl);

  void removeAllKeys();

 private:
  const proxy::IDatabaseSPtr db_;
};

class ExplorerKeyItem : public IExplorerTreeItem {
 public:
  ExplorerKeyItem(const core::NDbKValue& dbv,
                  const string_t& ns_separator,
                  proxy::NsDisplayStrategy ns_strategy,
                  IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  core::NDbKValue dbv() const;
  void setDbv(const core::NDbKValue& key);

  bool equalsKey(const core::NKey& key) const;

  core::NKey key() const;
  void setKey(const core::NKey& key);

  virtual string_t basicStringName() const override;
  proxy::IServerSPtr server() const;

  void renameKey(const QString& newName);
  void editValue(const core::NValue& value);
  void removeFromDb();
  void watchKey(int interval);
  void loadValueFromDb();
  void setTTL(core::ttl_t ttl);

  string_t nsSeparator() const;

 private:
  string_t fullName() const;

  core::NDbKValue dbv_;
  const string_t ns_separator_;
  const proxy::NsDisplayStrategy ns_strategy_;
};

class ExplorerNSItem : public IExplorerTreeItem {
 public:
  ExplorerNSItem(const string_t& name, const string_t& separator, IExplorerTreeItem* parent);
  ExplorerDatabaseItem* db() const;

  virtual string_t basicStringName() const override;
  proxy::IServerSPtr server() const;
  size_t keysCount() const;
  std::vector<const ExplorerKeyItem*> getKeys() const;
  string_t generateKeyTemplate(const string_t& key_name);

  void createKey(const core::NDbKValue& key);
  void removeBranch();

 private:
  const string_t name_;
  const string_t ns_separator_;
};

}  // namespace gui
}  // namespace fastonosql
