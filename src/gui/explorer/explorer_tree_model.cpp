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

#include "gui/explorer/explorer_tree_model.h"

#include <string>

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "core/icluster.h"
#include "core/isentinel.h"

#include "translations/global.h"

#include "gui/gui_factory.h"

namespace {
  const QString trDiscoveryToolTipTemplate_3S = QObject::tr("<b>Name:</b> %1<br/>"
                                                            "<b>Type:</b> %2<br/>"
                                                            "<b>Host:</b> %3<br/>");
  const QString trRemoteServerToolTipTemplate_4S = QObject::tr("<b>Name:</b> %1<br/>"
                                                               "<b>Type:</b> %2<br/>"
                                                               "<b>Mode:</b> %3<br/>"
                                                               "<b>Host:</b> %4<br/>");
  const QString trLocalServerToolTipTemplate_2S = QObject::tr("<b>Name:</b> %1<br/>"
                                                              "<b>Path:</b> %3<br/>");
  const QString trDbToolTipTemplate_1S = QObject::tr("<b>Db size:</b> %1 keys<br/>");
  const QString trNamespace_1S = QObject::tr("<b>Group size:</b> %1 keys<br/>");
}  // namespace

namespace fastonosql {
namespace gui {

IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent)
  : TreeItem(parent) {
}

ExplorerServerItem::ExplorerServerItem(core::IServerSPtr server, TreeItem* parent)
  : IExplorerTreeItem(parent), server_(server) {
}

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
  fasto::qt::gui::forEachRecursive(this, [&sz](const fasto::qt::gui::TreeItem* item) {
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

void ExplorerDatabaseItem::removeKey(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::CommandKeySPtr cmd(new core::CommandDeleteKey(key));
  core::events_info::CommandRequest req(this, dbs->info(), cmd);
  dbs->executeCommand(req);
}

void ExplorerDatabaseItem::loadValue(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::CommandKeySPtr cmd(new core::CommandLoadKey(key));
  core::events_info::CommandRequest req(this, dbs->info(), cmd);
  dbs->executeCommand(req);
}

void ExplorerDatabaseItem::createKey(const core::NDbKValue &key) {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::CommandKeySPtr cmd(new core::CommandCreateKey(key));
  core::events_info::CommandRequest req(this, dbs->info(), cmd);
  dbs->executeCommand(req);
}

void ExplorerDatabaseItem::removeAllKeys() {
  core::IDatabaseSPtr dbs = db();
  CHECK(dbs);
  core::events_info::ClearDatabaseRequest req(this, dbs->info());
  dbs->removeAllKeys(req);
}

ExplorerKeyItem::ExplorerKeyItem(const core::NDbKValue& key, IExplorerTreeItem *parent)
  : IExplorerTreeItem(parent), key_(key) {
}

ExplorerDatabaseItem* ExplorerKeyItem::db() const {
  TreeItem* par = parent();
  while(par) {
    ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(par);  // +
    if (db) {
      return db;
    }
    par = par->parent();
  }

  NOTREACHED();
  return nullptr;
}

core::NDbKValue ExplorerKeyItem::key() const {
  return key_;
}

QString ExplorerKeyItem::name() const {
  return common::ConvertFromString<QString>(key_.keyString());
}

core::IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  return par->server();
}

IExplorerTreeItem::eType ExplorerKeyItem::type() const {
  return eKey;
}

void ExplorerKeyItem::removeFromDb() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->removeKey(key_);
}

void ExplorerKeyItem::loadValueFromDb() {
  ExplorerDatabaseItem* par = db();
  CHECK(par);
  par->loadValue(key_);
}

ExplorerNSItem::ExplorerNSItem(const QString& name, IExplorerTreeItem* parent)
  : IExplorerTreeItem(parent), name_(name) {
}

QString ExplorerNSItem::name() const {
  return name_;
}

ExplorerDatabaseItem* ExplorerNSItem::db() const {
  TreeItem* par = parent();
  while(par) {
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
  fasto::qt::gui::forEachRecursive(this, [&sz](const fasto::qt::gui::TreeItem* item) {
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
  fasto::qt::gui::forEachRecursive(this, [par](fasto::qt::gui::TreeItem* item) {
    ExplorerKeyItem* key_item = dynamic_cast<ExplorerKeyItem*>(item);  // +
    if (!key_item) {
      return;
    }

    par->removeKey(key_item->key());
  });
}

ExplorerTreeModel::ExplorerTreeModel(QObject* parent)
  : TreeModel(parent) {
}

QVariant ExplorerTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  IExplorerTreeItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, IExplorerTreeItem*>(index);
  if (!node) {
    NOTREACHED();
    return QVariant();
  }

  int col = index.column();
  IExplorerTreeItem::eType type = node->type();

  if (role == Qt::ToolTipRole) {    
    if (type == IExplorerTreeItem::eServer) {
      ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);
      core::IServerSPtr server = server_node->server();
      core::ServerDiscoveryClusterInfoSPtr disc = server->discoveryClusterInfo();
      if (disc) {  // cluster
        QString dname = common::ConvertFromString<QString>(disc->name());
        QString dtype = common::ConvertFromString<QString>(common::ConvertToString(disc->type()));
        QString dhost = common::ConvertFromString<QString>(common::ConvertToString(disc->host()));
        return trDiscoveryToolTipTemplate_3S.arg(dname, dtype, dhost);
      }

      QString sname = common::ConvertFromString<QString>(server->name());
      bool isCanRemote = server->isCanRemote();
      if (isCanRemote) {
        core::IServerRemote* rserver = dynamic_cast<core::IServerRemote*>(server.get());  // +
        CHECK(rserver);
        QString stype = common::ConvertFromString<QString>(common::ConvertToString(rserver->role()));
        QString mtype = common::ConvertFromString<QString>(common::ConvertToString(rserver->mode()));
        QString shost = common::ConvertFromString<QString>(common::ConvertToString(rserver->host()));
        return trRemoteServerToolTipTemplate_4S.arg(sname, stype, mtype, shost);
      } else {
        core::IServerLocal* lserver = dynamic_cast<core::IServerLocal*>(server.get());  // +
        CHECK(lserver);
        QString spath = common::ConvertFromString<QString>(lserver->path());
        return trLocalServerToolTipTemplate_2S.arg(sname, spath);
      }
    } else if (type == IExplorerTreeItem::eDatabase) {
      ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);
      if (db->isDefault()) {
        return trDbToolTipTemplate_1S.arg(db->totalKeysCount());
      }
    } else if (type == IExplorerTreeItem::eNamespace) {
      ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);
      return trNamespace_1S.arg(ns->keyCount());
    }

    return QVariant();
  }

  if (role == Qt::DecorationRole && col == ExplorerServerItem::eName) {
    if (type == IExplorerTreeItem::eCluster) {
      return GuiFactory::instance().clusterIcon();
    } else if (type == IExplorerTreeItem::eSentinel) {
      return GuiFactory::instance().sentinelIcon();
    } else if (type == IExplorerTreeItem::eServer) {
      ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);
      core::IServerSPtr server = server_node->server();
      return GuiFactory::instance().icon(server->type());
    } else if (type == IExplorerTreeItem::eKey) {
      return GuiFactory::instance().keyIcon();
    } else if (type == IExplorerTreeItem::eDatabase) {
      return GuiFactory::instance().databaseIcon();
    } else if (type == IExplorerTreeItem::eNamespace) {
      return GuiFactory::instance().directoryIcon();
    } else {
      NOTREACHED();
    }
  }

  if (role == Qt::DisplayRole) {
    if (col == IExplorerTreeItem::eName) {
      if (type == IExplorerTreeItem::eKey) {
        return node->name();
      } else if (type == IExplorerTreeItem::eDatabase) {
        ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);
        return QString("%1 (%2/%3)").arg(node->name()).arg(db->loadedKeysCount()).arg(db->totalKeysCount());  // db
      } else if(type == IExplorerTreeItem::eNamespace) {
        ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);
        return QString("%1 (%2)").arg(node->name()).arg(ns->keyCount());  // db
      } else {
        return QString("%1 (%2)").arg(node->name()).arg(node->childrenCount());  // server, cluster
      }
    }
  }

  if (role == Qt::ForegroundRole) {
    if (type == IExplorerTreeItem::eDatabase) {
      ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);
      if (db->isDefault()) {
        return QVariant(QColor(Qt::red));
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags ExplorerTreeModel::flags(const QModelIndex& index) const {
  if (index.isValid()) {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }

  return Qt::NoItemFlags;
}

QVariant ExplorerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == ExplorerServerItem::eName) {
      return translations::trName;
    }
  }

  return TreeModel::headerData(section, orientation, role);
}

int ExplorerTreeModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return ExplorerServerItem::eCountColumns;
}

void ExplorerTreeModel::addCluster(core::IClusterSPtr cluster) {
  ExplorerClusterItem* cl = findClusterItem(cluster);
  if (!cl) {
    fasto::qt::gui::TreeItem* parent = root_;
    CHECK(parent);

    ExplorerClusterItem* item = new ExplorerClusterItem(cluster, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeCluster(core::IClusterSPtr cluster) {
  ExplorerClusterItem* serverItem = findClusterItem(cluster);
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addServer(core::IServerSPtr server) {
  if (!server) {
    return;
  }

  ExplorerServerItem* serv = findServerItem(server.get());
  if (!serv) {
    fasto::qt::gui::TreeItem* parent = root_;
    CHECK(parent);

    ExplorerServerItem* item = new ExplorerServerItem(server, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeServer(core::IServerSPtr server) {
  ExplorerServerItem* serverItem = findServerItem(server.get());
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addSentinel(core::ISentinelSPtr sentinel) {
  ExplorerSentinelItem* cl = findSentinelItem(sentinel);
  if (!cl) {
    fasto::qt::gui::TreeItem* parent = root_;
    CHECK(parent);

    ExplorerSentinelItem* item = new ExplorerSentinelItem(sentinel, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeSentinel(core::ISentinelSPtr sentinel) {
  ExplorerSentinelItem* serverItem = findSentinelItem(sentinel);
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addDatabase(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    fasto::qt::gui::TreeItem* parent_server = parent->parent();
    QModelIndex parent_index = createIndex(parent_server->indexOf(parent), 0, parent);
    ExplorerDatabaseItem* item = new ExplorerDatabaseItem(server->createDatabaseByInfo(db), parent);
    insertItem(parent_index, item);
  }
}

void ExplorerTreeModel::removeDatabase(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (dbs) {
    fasto::qt::gui::TreeItem* parent_server = parent->parent();
    QModelIndex index = createIndex(parent_server->indexOf(parent), 0, dbs);
    removeItem(index.parent(), dbs);
  }
}

void ExplorerTreeModel::setDefaultDb(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    DNOTREACHED();
    return;
  }

  QModelIndex parent_index = createIndex(root_->indexOf(parent), ExplorerDatabaseItem::eName, parent);
  QModelIndex dbs_last_index = index(parent->childrenCount(), ExplorerDatabaseItem::eCountColumns, parent_index);
  updateItem(parent_index, dbs_last_index);
}

void ExplorerTreeModel::updateDb(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  CHECK(dbs);

  int index_db = parent->indexOf(dbs);
  QModelIndex dbs_index1 = createIndex(index_db, ExplorerDatabaseItem::eName, dbs);
  QModelIndex dbs_index2 = createIndex(index_db, ExplorerDatabaseItem::eCountColumns, dbs);
  updateItem(dbs_index1, dbs_index2);
}

void ExplorerTreeModel::addKey(core::IServer* server, core::IDataBaseInfoSPtr db,
                               const core::NDbKValue &dbv, const std::string& ns_separator) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  CHECK(dbs);

  ExplorerKeyItem* keyit = findKeyItem(dbs, dbv);
  if (!keyit) {
    IExplorerTreeItem* nitem = dbs;
    core::NKey key = dbv.key();
    core::KeyInfo kinf = key.info(ns_separator);
    if (kinf.hasNamespace()) {
      nitem = findOrCreateNSItem(dbs, kinf);
      CHECK(nitem);
    }

    fasto::qt::gui::TreeItem* parent_nitem = nitem->parent();
    QModelIndex parent_index = createIndex(parent_nitem->indexOf(nitem), 0, nitem);
    ExplorerKeyItem* item = new ExplorerKeyItem(dbv, nitem);
    insertItem(parent_index, item);
  }
}

void ExplorerTreeModel::removeKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue& key) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  CHECK(dbs);

  ExplorerKeyItem* keyit = findKeyItem(dbs, key);
  if (keyit) {
    fasto::qt::gui::TreeItem* par = keyit->parent();
    QModelIndex index = createIndex(par->indexOf(keyit), 0, keyit);
    removeItem(index.parent(), keyit);
  }
}

void ExplorerTreeModel::removeAllKeys(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  CHECK(parent);

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  CHECK(dbs);

  QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
  removeAllItems(parentdb);
}

ExplorerClusterItem* ExplorerTreeModel::findClusterItem(core::IClusterSPtr cl) {
  fasto::qt::gui::TreeItem* parent = root_;
  CHECK(parent);

  for (size_t i = 0; i < parent->childrenCount() ; ++i) {
    ExplorerClusterItem* item = dynamic_cast<ExplorerClusterItem*>(parent->child(i));  // +
    if (item && item->cluster() == cl) {
      return item;
    }
  }
  return nullptr;
}

ExplorerSentinelItem* ExplorerTreeModel::findSentinelItem(core::ISentinelSPtr sentinel) {
  fasto::qt::gui::TreeItem* parent = root_;
  CHECK(parent);

  for (size_t i = 0; i < parent->childrenCount() ; ++i) {
    ExplorerSentinelItem* item = dynamic_cast<ExplorerSentinelItem*>(parent->child(i));  // +
    if (item && item->sentinel() == sentinel) {
      return item;
    }
  }
  return nullptr;
}

ExplorerServerItem* ExplorerTreeModel::findServerItem(core::IServer* server) const {  
  return static_cast<ExplorerServerItem*>(fasto::qt::gui::findItemRecursive(root_, [server](fasto::qt::gui::TreeItem* item) -> bool
  {
    ExplorerServerItem* server_item = dynamic_cast<ExplorerServerItem*>(item);  // +
    if (!server_item) {
      return false;
    }

    return server_item->server().get() == server;
  }));
}

ExplorerDatabaseItem* ExplorerTreeModel::findDatabaseItem(ExplorerServerItem* server,
                                                          core::IDataBaseInfoSPtr db) const {
  if (!server) {
    DNOTREACHED();
    return nullptr;
  }

  for (size_t i = 0; i < server->childrenCount() ; ++i) {
    ExplorerDatabaseItem* item = dynamic_cast<ExplorerDatabaseItem*>(server->child(i));  // +
    CHECK(item);

    core::IDatabaseSPtr inf = item->db();
    if (inf && inf->name() == db->name()) {
      return item;
    }
  }

  return nullptr;
}

ExplorerKeyItem* ExplorerTreeModel::findKeyItem(IExplorerTreeItem* db_or_ns,
                                                const core::NDbKValue& key) const {
  return static_cast<ExplorerKeyItem*>(fasto::qt::gui::findItemRecursive(db_or_ns, [key](fasto::qt::gui::TreeItem* item) -> bool
  {
    ExplorerKeyItem* key_item = dynamic_cast<ExplorerKeyItem*>(item);  // +
    if (!key_item) {
      return false;
    }

    core::NDbKValue ckey = key_item->key();
    return ckey.keyString() == key.keyString();
  }));
}

ExplorerNSItem* ExplorerTreeModel::findNSItem(IExplorerTreeItem* db_or_ns, const QString& name) const {
  return static_cast<ExplorerNSItem*>(fasto::qt::gui::findItemRecursive(db_or_ns, [name](fasto::qt::gui::TreeItem* item) -> bool
  {
    ExplorerNSItem* ns_item = dynamic_cast<ExplorerNSItem*>(item);  // +
    if (!ns_item) {
      return false;
    }

    return ns_item->name() == name;
  }));
}

ExplorerNSItem* ExplorerTreeModel::findOrCreateNSItem(IExplorerTreeItem* db_or_ns, const core::KeyInfo& kinf) {
  std::string nspace = kinf.nspace();
  QString qnspace = common::ConvertFromString<QString>(nspace);
  ExplorerNSItem* founded_item = findNSItem(db_or_ns, qnspace);
  if (founded_item) {
    return founded_item;
  }

  size_t sz = kinf.nspaceSize();
  IExplorerTreeItem* par = db_or_ns;
  for (size_t i = 0; i < sz; ++i) {
    ExplorerNSItem* item = nullptr;
    nspace = kinf.joinNamespace(i);
    qnspace = common::ConvertFromString<QString>(nspace);
    for (size_t j = 0; j < par->childrenCount(); ++j) {
      ExplorerNSItem* ns_item = dynamic_cast<ExplorerNSItem*>(par->child(j));  // +
      if (!ns_item) {
        continue;
      }

      if (ns_item->name() == qnspace) {
        item = ns_item;
      }
    }

    if (!item) {
      fasto::qt::gui::TreeItem* gpar = par->parent();
      QModelIndex parentdb = createIndex(gpar->indexOf(par), 0, par);
      item = new ExplorerNSItem(qnspace, par);
      insertItem(parentdb, item);
    }

    par = item;
    founded_item = item;
  }

  return founded_item;
}

}  // namespace gui
}  // namespace fastonosql
