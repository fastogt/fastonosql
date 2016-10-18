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

#include "core/connection_types.h"       // for ConvertToString
#include "core/events/events_info.h"     // for CommandRequest, etc
#include "core/database/idatabase.h"     // for IDatabase
#include "core/server/iserver_local.h"   // for IServer, IServerRemote, etc
#include "core/server/iserver_remote.h"  // for IServer, IServerRemote, etc
#include "core/cluster/icluster.h"       // for ICluster
#include "core/sentinel/isentinel.h"     // for ISentinel, Sentinel, etc

#include "gui/gui_factory.h"  // for GuiFactory

namespace fastonosql {
namespace gui {
IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent) : TreeItem(parent, nullptr) {}

ExplorerServerItem::ExplorerServerItem(core::IServerSPtr server, TreeItem* parent)
    : IExplorerTreeItem(parent), server_(server) {}

QString ExplorerServerItem::name() const {
  return common::ConvertFromString<QString>(server_->name());
}

core::IServerSPtr ExplorerServerItem::server() const {
  return server_;
}

ExplorerServerItem::eType ExplorerServerItem::type() const {
  return eServer;
}

void ExplorerServerItem::loadDatabases() {
  core::events_info::LoadDatabasesInfoRequest req(this);
  return server_->loadDatabases(req);
}

ExplorerSentinelItem::ExplorerSentinelItem(core::ISentinelSPtr sentinel, TreeItem* parent)
    : IExplorerTreeItem(parent), sentinel_(sentinel) {
  core::ISentinel::sentinels_t nodes = sentinel->sentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    core::Sentinel sent = nodes[i];
    ExplorerServerItem* rser = new ExplorerServerItem(sent.sentinel, this);
    addChildren(rser);

    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      ExplorerServerItem* ser = new ExplorerServerItem(sent.sentinels_nodes[j], rser);
      rser->addChildren(ser);
    }
  }
}

QString ExplorerSentinelItem::name() const {
  return common::ConvertFromString<QString>(sentinel_->name());
}

ExplorerSentinelItem::eType ExplorerSentinelItem::type() const {
  return eSentinel;
}

core::ISentinelSPtr ExplorerSentinelItem::sentinel() const {
  return sentinel_;
}

ExplorerClusterItem::ExplorerClusterItem(core::IClusterSPtr cluster, TreeItem* parent)
    : IExplorerTreeItem(parent), cluster_(cluster) {
  auto nodes = cluster_->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
    addChildren(ser);
  }
}

QString ExplorerClusterItem::name() const {
  return common::ConvertFromString<QString>(cluster_->name());
}

ExplorerClusterItem::eType ExplorerClusterItem::type() const {
  return eCluster;
}

core::IClusterSPtr ExplorerClusterItem::cluster() const {
  return cluster_;
}

ExplorerDatabaseItem::ExplorerDatabaseItem(core::IDatabaseSPtr db, ExplorerServerItem* parent)
    : IExplorerTreeItem(parent), db_(db) {
  DCHECK(db_);
}

QString ExplorerDatabaseItem::name() const {
  return common::ConvertFromString<QString>(db_->name());
}

ExplorerDatabaseItem::eType ExplorerDatabaseItem::type() const {
  return eDatabase;
}

bool ExplorerDatabaseItem::isDefault() const {
  return info()->isDefault();
}

size_t ExplorerDatabaseItem::totalKeysCount() const {
  core::IDataBaseInfoSPtr inf = info();
  return inf->dbKeysCount();
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

core::IServerSPtr ExplorerDatabaseItem::server() const {
  CHECK(db_);
  return db_->server();
}

core::IDatabaseSPtr ExplorerDatabaseItem::db() const {
  CHECK(db_);
  return db_;
}

void ExplorerDatabaseItem::loadContent(const std::string& pattern, uint32_t countKeys) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::events_info::LoadDatabaseContentRequest req(this, dbs->info(), pattern, countKeys);
  dbs->loadContent(req);
}

void ExplorerDatabaseItem::setDefault() {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::events_info::SetDefaultDatabaseRequest req(this, dbs->info());
  dbs->setDefault(req);
}

core::IDataBaseInfoSPtr ExplorerDatabaseItem::info() const {
  return db_->info();
}

void ExplorerDatabaseItem::renameKey(const core::NKey& key, const QString& newName) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->renameKeyCommand(key, common::ConvertToString(newName), &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::removeKey(const core::NKey& key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->deleteKeyCommand(key, &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::loadValue(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->loadKeyCommand(key.key(), key.type(), &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::watchKey(const core::NDbKValue& key, int interval) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->loadKeyCommand(key.key(), key.type(), &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str, std::numeric_limits<size_t>::max() - 1,
                                            interval, false);
  dbs->execute(req);
}

void ExplorerDatabaseItem::createKey(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->createKeyCommand(key, &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::editKey(const core::NDbKValue& key, const core::NValue& value) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  core::NDbKValue copy_key = key;
  copy_key.setValue(value);
  common::Error err = tran->createKeyCommand(copy_key, &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::setTTL(const core::NKey& key, core::ttl_t ttl) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::translator_t tran = dbs->translator();
  std::string cmd_str;
  common::Error err = tran->changeKeyTTLCommand(key, ttl, &cmd_str);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  dbs->execute(req);
}

void ExplorerDatabaseItem::removeAllKeys() {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::events_info::ClearDatabaseRequest req(this, dbs->info());
  dbs->removeAllKeys(req);
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
  return dbv_.key();
}

void ExplorerKeyItem::setKey(const core::NKey& key) {
  dbv_.setKey(key);
}

QString ExplorerKeyItem::name() const {
  return common::ConvertFromString<QString>(dbv_.keyString());
}

core::IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  return par->server();
}

IExplorerTreeItem::eType ExplorerKeyItem::type() const {
  return eKey;
}

void ExplorerKeyItem::renameKey(const QString& newName) {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->renameKey(dbv_.key(), newName);
}

void ExplorerKeyItem::editKey(const core::NValue& value) {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->editKey(dbv_, value);
}

void ExplorerKeyItem::removeFromDb() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->removeKey(dbv_.key());
}

void ExplorerKeyItem::watchKey(int interval) {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->watchKey(dbv_, interval);
}

void ExplorerKeyItem::loadValueFromDb() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->loadValue(dbv_);
}

void ExplorerKeyItem::setTTL(core::ttl_t ttl) {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->setTTL(dbv_.key(), ttl);
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

core::IServerSPtr ExplorerNSItem::server() const {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  return par->server();
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
