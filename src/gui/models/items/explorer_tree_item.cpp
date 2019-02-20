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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/models/items/explorer_tree_item.h"

#include <limits>
#include <string>
#include <vector>

#include <common/convert2string.h>

#include <common/qt/convert2string.h>
#include <common/qt/logger.h>

#include "proxy/database/idatabase.h"

#include "proxy/cluster/icluster.h"
#include "proxy/sentinel/isentinel.h"
#include "proxy/server/iserver.h"

#include "gui/key_info.h"

namespace fastonosql {
namespace gui {

IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent, eType type) : TreeItem(parent, nullptr), type_(type) {}

ExplorerServerItem::eType IExplorerTreeItem::type() const {
  return type_;
}

ExplorerServerItem::ExplorerServerItem(proxy::IServerSPtr server, TreeItem* parent)
    : IExplorerTreeItem(parent, eServer), server_(server) {}

QString ExplorerServerItem::name() const {
  QString qname;
  common::ConvertFromString(server_->GetName(), &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerServerItem::basicStringName() const {
  return common::ConvertToCharBytes(server_->GetName());
}

proxy::IServerSPtr ExplorerServerItem::server() const {
  return server_;
}

void ExplorerServerItem::loadDatabases() {
  proxy::events_info::LoadDatabasesInfoRequest req(this);
  return server_->LoadDatabases(req);
}

void ExplorerServerItem::createDatabase(const QString& name) {
  core::translator_t tran = server_->GetTranslator();
  core::command_buffer_t cmd_str;
  const auto name_str = common::ConvertToCharBytes(name);
  common::Error err = tran->CreateDBCommand(name_str, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  server_->Execute(req);
}

void ExplorerServerItem::removeDatabase(const QString& name) {
  core::translator_t tran = server_->GetTranslator();
  core::command_buffer_t cmd_str;
  const auto name_str = common::ConvertToCharBytes(name);
  common::Error err = tran->RemoveDBCommand(name_str, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  server_->Execute(req);
}

#if defined(PRO_VERSION)
ExplorerSentinelItem::ExplorerSentinelItem(proxy::ISentinelSPtr sentinel, TreeItem* parent)
    : IExplorerTreeItem(parent, eSentinel), sentinel_(sentinel) {
  proxy::ISentinel::sentinels_t nodes = sentinel->GetSentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    proxy::Sentinel sent = nodes[i];
    ExplorerServerItem* rser = new ExplorerServerItem(sent.sentinel, this);
    addChildren(rser);

    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      ExplorerServerItem* ser = new ExplorerServerItem(sent.sentinels_nodes[j], rser);
      rser->addChildren(ser);
    }
  }
}

QString ExplorerSentinelItem::name() const {
  QString qname;
  common::ConvertFromString(sentinel_->GetName(), &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerSentinelItem::basicStringName() const {
  return common::ConvertToCharBytes(sentinel_->GetName());
}

proxy::ISentinelSPtr ExplorerSentinelItem::sentinel() const {
  return sentinel_;
}

ExplorerClusterItem::ExplorerClusterItem(proxy::IClusterSPtr cluster, TreeItem* parent)
    : IExplorerTreeItem(parent, eCluster), cluster_(cluster) {
  auto nodes = cluster_->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
    addChildren(ser);
  }
}

QString ExplorerClusterItem::name() const {
  QString qname;
  common::ConvertFromString(cluster_->GetName(), &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerClusterItem::basicStringName() const {
  return common::ConvertToCharBytes(cluster_->GetName());
}

proxy::IClusterSPtr ExplorerClusterItem::cluster() const {
  return cluster_;
}
#endif

ExplorerDatabaseItem::ExplorerDatabaseItem(proxy::IDatabaseSPtr db, ExplorerServerItem* parent)
    : IExplorerTreeItem(parent, eDatabase), db_(db) {
  DCHECK(db_);
}

QString ExplorerDatabaseItem::name() const {
  QString qname;
  common::ConvertFromBytes(db_->GetName(), &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerDatabaseItem::basicStringName() const {
  return db_->GetName();
}

bool ExplorerDatabaseItem::isDefault() const {
  return db_->IsDefault();
}

size_t ExplorerDatabaseItem::totalKeysCount() const {
  core::IDataBaseInfoSPtr inf = db_->GetInfo();
  return inf->GetDBKeysCount();
}

size_t ExplorerDatabaseItem::loadedKeysCount() const {
  size_t sz = 0;
  common::qt::gui::forEachRecursive(this, [&sz](const common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = static_cast<const ExplorerKeyItem*>(item);
    if (key_item->type() != eKey) {
      return;
    }

    sz++;
  });

  return sz;
}

proxy::IServerSPtr ExplorerDatabaseItem::server() const {
  return db_->GetServer();
}

proxy::IDatabaseSPtr ExplorerDatabaseItem::db() const {
  return db_;
}

void ExplorerDatabaseItem::loadContent(const core::pattern_t& pattern, core::keys_limit_t keys_count) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::events_info::LoadDatabaseContentRequest req(this, dbs->GetInfo(), pattern, keys_count);
  dbs->LoadContent(req);
}

void ExplorerDatabaseItem::setDefault() {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);

  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->SelectDBCommand(dbs->GetName(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::removeDb() {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);

  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->RemoveDBCommand(dbs->GetName(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::renameKey(const core::NKey& key, const QString& new_name) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  const core::nkey_t key_str(common::ConvertToCharBytes(new_name));  // convert from QString
  common::Error err = tran->RenameKeyCommand(key, key_str, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::removeKey(const core::NKey& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->DeleteKeyCommand(key, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::loadValue(const core::NDbKValue& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->LoadKeyCommand(key.GetKey(), key.GetType(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::loadType(const core::NDbKValue& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->GetTypeCommand(key.GetKey(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::watchKey(const core::NDbKValue& key, int interval) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->LoadKeyCommand(key.GetKey(), key.GetType(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str, std::numeric_limits<size_t>::max() - 1, interval, false);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::createKey(const core::NDbKValue& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;

  common::Error err = tran->CreateKeyCommand(key, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::editValue(const core::NDbKValue& key, const core::NValue& value) {
  core::NDbKValue copy_key = key;
  copy_key.SetValue(value);

  createKey(copy_key);
}

void ExplorerDatabaseItem::setTTL(const core::NKey& key, core::ttl_t ttl) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::removeAllKeys() {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->FlushDBCommand(&cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

ExplorerKeyItem::ExplorerKeyItem(const core::NDbKValue& dbv,
                                 const std::string& ns_separator,
                                 proxy::NsDisplayStrategy ns_strategy,
                                 IExplorerTreeItem* parent)
    : IExplorerTreeItem(parent, eKey), dbv_(dbv), ns_separator_(ns_separator), ns_strategy_(ns_strategy) {}

ExplorerDatabaseItem* ExplorerKeyItem::db() const {
  TreeItem* par = parent();
  while (par) {
    IExplorerTreeItem* item = static_cast<IExplorerTreeItem*>(par);
    if (item->type() == eDatabase) {
      return static_cast<ExplorerDatabaseItem*>(item);
    }
    par = par->parent();
  }

  NOTREACHED();
  return nullptr;
}

core::NDbKValue ExplorerKeyItem::dbv() const {
  return dbv_;
}

void ExplorerKeyItem::setDbv(const core::NDbKValue& key) {
  dbv_ = key;
}

bool ExplorerKeyItem::equalsKey(const core::NKey& key) const {
  return dbv_.EqualsKey(key);
}

core::NKey ExplorerKeyItem::key() const {
  return dbv_.GetKey();
}

void ExplorerKeyItem::setKey(const core::NKey& key) {
  dbv_.SetKey(key);
}

QString ExplorerKeyItem::name() const {
  QString qname;
  common::ConvertFromBytes(basicStringName(), &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerKeyItem::basicStringName() const {
  if (ns_strategy_ == proxy::FULL_KEY) {
    return fullName();
  }

  const core::NKey key = dbv_.GetKey();
  const auto key_str = key.GetKey();
  KeyInfo kinf(key_str.GetHumanReadable(), ns_separator_);
  if (!kinf.hasNamespace()) {
    return fullName();
  }

  return kinf.keyName();
}

proxy::IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* par = db();
  if (par) {
    return par->server();
  }

  return proxy::IServerSPtr();
}

void ExplorerKeyItem::renameKey(const QString& newName) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->renameKey(dbv_.GetKey(), newName);
  }
}

void ExplorerKeyItem::editValue(const core::NValue& value) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->editValue(dbv_, value);
  }
}

void ExplorerKeyItem::removeFromDb() {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->removeKey(dbv_.GetKey());
  }
}

void ExplorerKeyItem::watchKey(int interval) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->watchKey(dbv_, interval);
  }
}

void ExplorerKeyItem::loadValueFromDb() {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->loadValue(dbv_);
  }
}

void ExplorerKeyItem::loadTypeFromDb() {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->loadType(dbv_);
  }
}

void ExplorerKeyItem::setTTL(core::ttl_t ttl) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->setTTL(dbv_.GetKey(), ttl);
  }
}

std::string ExplorerKeyItem::nsSeparator() const {
  return ns_separator_;
}

ExplorerKeyItem::string_t ExplorerKeyItem::fullName() const {
  const core::NKey key = dbv_.GetKey();
  const auto nkey = key.GetKey();
  return nkey.GetHumanReadable();
}

ExplorerNSItem::ExplorerNSItem(const string_t& name, const std::string& separator, IExplorerTreeItem* parent)
    : IExplorerTreeItem(parent, eNamespace), name_(name), ns_separator_(separator) {}

QString ExplorerNSItem::name() const {
  QString qname;
  common::ConvertFromBytes(name_, &qname);
  return qname;
}

IExplorerTreeItem::string_t ExplorerNSItem::basicStringName() const {
  return name_;
}

ExplorerDatabaseItem* ExplorerNSItem::db() const {
  TreeItem* par = parent();
  while (par) {
    IExplorerTreeItem* item = static_cast<IExplorerTreeItem*>(par);
    if (item->type() == eDatabase) {
      return static_cast<ExplorerDatabaseItem*>(item);
    }
    par = par->parent();
  }

  NOTREACHED();
  return nullptr;
}

proxy::IServerSPtr ExplorerNSItem::server() const {
  ExplorerDatabaseItem* par = db();
  if (par) {
    return par->server();
  }

  return proxy::IServerSPtr();
}

size_t ExplorerNSItem::keysCount() const {
  std::vector<const ExplorerKeyItem*> keys = getKeys();
  return keys.size();
}

std::vector<const ExplorerKeyItem*> ExplorerNSItem::getKeys() const {
  std::vector<const ExplorerKeyItem*> keys;
  common::qt::gui::forEachRecursive(this, [&keys](const common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = static_cast<const ExplorerKeyItem*>(item);
    if (key_item->type() != eKey) {
      return;
    }

    keys.push_back(key_item);
  });

  return keys;
}

IExplorerTreeItem::string_t ExplorerNSItem::generateKeyTemplate(const string_t& key_name) {
  TreeItem* par = parent();
  string_t key_name_new = basicStringName();
  key_name_new += ns_separator_;
  key_name_new += key_name;
  while (par) {
    IExplorerTreeItem* item = static_cast<IExplorerTreeItem*>(par);
    if (item->type() == eDatabase) {
      return key_name_new;
    }

    CHECK(item->type() == eNamespace);
    core::command_buffer_writer_t wr;
    wr << item->basicStringName() << ns_separator_ << key_name_new;
    key_name_new = wr.str();
    par = par->parent();
  }

  DNOTREACHED();
  return key_name_new;
}

void ExplorerNSItem::createKey(const core::NDbKValue& key) {
  ExplorerDatabaseItem* database = db();
  database->createKey(key);
}

void ExplorerNSItem::removeBranch() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  common::qt::gui::forEachRecursive(this, [par](common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = static_cast<const ExplorerKeyItem*>(item);
    if (key_item->type() != eKey) {
      return;
    }

    par->removeKey(key_item->key());
  });
}

void ExplorerNSItem::renameBranch(const QString& old_branch_name, const QString& new_branch_name) {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  common::qt::gui::forEachRecursive(this, [par, old_branch_name, new_branch_name](common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = static_cast<const ExplorerKeyItem*>(item);
    if (key_item->type() != eKey) {
      return;
    }

    QString key_name = key_item->name();
    key_name = key_name.replace(old_branch_name, new_branch_name);
    par->renameKey(key_item->key(), key_name);
  });
}

}  // namespace gui
}  // namespace fastonosql
