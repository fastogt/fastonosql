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

#include "gui/explorer/explorer_tree_item.h"

#include <memory>  // for __shared_ptr, operator==, etc
#include <string>  // for operator==, string, etc
#include <vector>  // for vector

#include <QIcon>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for CHECK, NOTREACHED, etc
#include <common/net/types.h>       // for ConvertToString

#include <common/qt/utils_qt.h>  // for item
#include <common/qt/convert2string.h>
#include <common/qt/gui/base/tree_item.h>   // for TreeItem, findItemRecursive, etc
#include <common/qt/gui/base/tree_model.h>  // for TreeModel
#include <common/qt/logger.h>

#include "core/connection_types.h"     // for ConvertToString
#include "proxy/events/events_info.h"  // for CommandRequest, etc
#include "proxy/database/idatabase.h"  // for IDatabase

#include "proxy/server/iserver_local.h"   // for IServer, IServerRemote, etc
#include "proxy/server/iserver_remote.h"  // for IServer, IServerRemote, etc
#include "proxy/cluster/icluster.h"       // for ICluster
#include "proxy/sentinel/isentinel.h"     // for ISentinel, Sentinel, etc

#include "gui/gui_factory.h"  // for GuiFactory

namespace fastonosql {
namespace gui {
IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent) : TreeItem(parent, nullptr) {}

ExplorerServerItem::ExplorerServerItem(proxy::IServerSPtr server, TreeItem* parent)
    : IExplorerTreeItem(parent), server_(server) {}

QString ExplorerServerItem::name() const {
  QString qname;
  common::ConvertFromString(server_->Name(), &qname);
  return qname;
}

proxy::IServerSPtr ExplorerServerItem::server() const {
  return server_;
}

ExplorerServerItem::eType ExplorerServerItem::type() const {
  return eServer;
}

void ExplorerServerItem::loadDatabases() {
  proxy::events_info::LoadDatabasesInfoRequest req(this);
  return server_->LoadDatabases(req);
}

ExplorerSentinelItem::ExplorerSentinelItem(proxy::ISentinelSPtr sentinel, TreeItem* parent)
    : IExplorerTreeItem(parent), sentinel_(sentinel) {
  proxy::ISentinel::sentinels_t nodes = sentinel->Sentinels();
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
  common::ConvertFromString(sentinel_->Name(), &qname);
  return qname;
}

ExplorerSentinelItem::eType ExplorerSentinelItem::type() const {
  return eSentinel;
}

proxy::ISentinelSPtr ExplorerSentinelItem::sentinel() const {
  return sentinel_;
}

ExplorerClusterItem::ExplorerClusterItem(proxy::IClusterSPtr cluster, TreeItem* parent)
    : IExplorerTreeItem(parent), cluster_(cluster) {
  auto nodes = cluster_->Nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
    addChildren(ser);
  }
}

QString ExplorerClusterItem::name() const {
  QString qname;
  common::ConvertFromString(cluster_->Name(), &qname);
  return qname;
}

ExplorerClusterItem::eType ExplorerClusterItem::type() const {
  return eCluster;
}

proxy::IClusterSPtr ExplorerClusterItem::cluster() const {
  return cluster_;
}

ExplorerDatabaseItem::ExplorerDatabaseItem(proxy::IDatabaseSPtr db, ExplorerServerItem* parent)
    : IExplorerTreeItem(parent), db_(db) {
  DCHECK(db_);
}

QString ExplorerDatabaseItem::name() const {
  QString qname;
  common::ConvertFromString(db_->Name(), &qname);
  return qname;
}

ExplorerDatabaseItem::eType ExplorerDatabaseItem::type() const {
  return eDatabase;
}

bool ExplorerDatabaseItem::isDefault() const {
  return info()->IsDefault();
}

size_t ExplorerDatabaseItem::totalKeysCount() const {
  core::IDataBaseInfoSPtr inf = info();
  return inf->DBKeysCount();
}

size_t ExplorerDatabaseItem::loadedKeysCount() const {
  size_t sz = 0;
  common::qt::gui::forEachRecursive(this, [&sz](const common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = dynamic_cast<const ExplorerKeyItem*>(item);  // +
    if (!key_item) {
      return;
    }

    sz++;
  });

  return sz;
}

proxy::IServerSPtr ExplorerDatabaseItem::server() const {
  CHECK(db_);
  return db_->Server();
}

proxy::IDatabaseSPtr ExplorerDatabaseItem::db() const {
  CHECK(db_);
  return db_;
}

void ExplorerDatabaseItem::loadContent(const std::string& pattern, uint32_t countKeys) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::events_info::LoadDatabaseContentRequest req(this, dbs->Info(), pattern, countKeys);
  dbs->LoadContent(req);
}

void ExplorerDatabaseItem::setDefault() {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);

  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->SelectDBCommand(dbs->Name(), &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

core::IDataBaseInfoSPtr ExplorerDatabaseItem::info() const {
  return db_->Info();
}

void ExplorerDatabaseItem::renameKey(const core::NKey& key, const QString& newName) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->RenameKeyCommand(key, common::ConvertToString(newName), &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::removeKey(const core::NKey& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->DeleteKeyCommand(key, &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::loadValue(const core::NDbKValue& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->LoadKeyCommand(key.Key(), key.Type(), &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::watchKey(const core::NDbKValue& key, int interval) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->LoadKeyCommand(key.Key(), key.Type(), &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str, std::numeric_limits<size_t>::max() - 1,
                                             interval, false);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::createKey(const core::NDbKValue& key) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->CreateKeyCommand(key, &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::editKey(const core::NDbKValue& key, const core::NValue& value) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  core::NDbKValue copy_key = key;
  copy_key.SetValue(value);
  common::Error err = tran->CreateKeyCommand(copy_key, &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::setTTL(const core::NKey& key, core::ttl_t ttl) {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

void ExplorerDatabaseItem::removeAllKeys() {
  proxy::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  proxy::IServerSPtr server = dbs->Server();
  core::translator_t tran = server->Translator();
  std::string cmd_str;
  common::Error err = tran->FlushDBCommand(&cmd_str);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->Execute(req);
}

ExplorerKeyItem::ExplorerKeyItem(const core::NDbKValue& dbv, IExplorerTreeItem* parent)
    : IExplorerTreeItem(parent), dbv_(dbv) {}

ExplorerDatabaseItem* ExplorerKeyItem::db() const {
  TreeItem* par = parent();
  while (par) {
    ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(par);  // +
    if (db) {
      return db;
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

core::NKey ExplorerKeyItem::key() const {
  return dbv_.Key();
}

void ExplorerKeyItem::setKey(const core::NKey& key) {
  dbv_.SetKey(key);
}

QString ExplorerKeyItem::name() const {
  QString qname;
  common::ConvertFromString(dbv_.KeyString(), &qname);
  return qname;
}

proxy::IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* par = db();
  if (par) {
    return par->server();
  }

  return proxy::IServerSPtr();
}

IExplorerTreeItem::eType ExplorerKeyItem::type() const {
  return eKey;
}

void ExplorerKeyItem::renameKey(const QString& newName) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->renameKey(dbv_.Key(), newName);
  }
}

void ExplorerKeyItem::editKey(const core::NValue& value) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->editKey(dbv_, value);
  }
}

void ExplorerKeyItem::removeFromDb() {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->removeKey(dbv_.Key());
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

void ExplorerKeyItem::setTTL(core::ttl_t ttl) {
  ExplorerDatabaseItem* par = db();
  if (par) {
    par->setTTL(dbv_.Key(), ttl);
  }
}

ExplorerNSItem::ExplorerNSItem(const QString& name, IExplorerTreeItem* parent)
    : IExplorerTreeItem(parent), name_(name) {}

QString ExplorerNSItem::name() const {
  return name_;
}

ExplorerDatabaseItem* ExplorerNSItem::db() const {
  TreeItem* par = parent();
  while (par) {
    ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(par);  // +
    if (db) {
      return db;
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

ExplorerNSItem::eType ExplorerNSItem::type() const {
  return eNamespace;
}

size_t ExplorerNSItem::keyCount() const {
  size_t sz = 0;
  common::qt::gui::forEachRecursive(this, [&sz](const common::qt::gui::TreeItem* item) {
    const ExplorerKeyItem* key_item = dynamic_cast<const ExplorerKeyItem*>(item);  // +
    if (!key_item) {
      return;
    }

    sz++;
  });

  return sz;
}

void ExplorerNSItem::removeBranch() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  common::qt::gui::forEachRecursive(this, [par](common::qt::gui::TreeItem* item) {
    ExplorerKeyItem* key_item = dynamic_cast<ExplorerKeyItem*>(item);  // +
    if (!key_item) {
      return;
    }

    par->removeKey(key_item->key());
  });
}
}  // namespace gui
}  // namespace fastonosql
