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

#include "translations/global.h"

#include "gui/gui_factory.h"

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "core/icluster.h"

namespace fastonosql {
namespace gui {

IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent)
  : TreeItem(parent) {
}

ExplorerServerItem::ExplorerServerItem(core::IServerSPtr server, TreeItem* parent)
  : IExplorerTreeItem(parent), server_(server) {
}

QString ExplorerServerItem::name() const {
  return common::convertFromString<QString>(server_->name());
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

ExplorerClusterItem::ExplorerClusterItem(core::IClusterSPtr cluster, TreeItem* parent)
  : IExplorerTreeItem(parent), cluster_(cluster) {
  core::ICluster::nodes_type nodes = cluster_->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
    addChildren(ser);
  }
}

QString ExplorerClusterItem::name() const {
  return common::convertFromString<QString>(cluster_->name());
}

core::IServerSPtr ExplorerClusterItem::server() const {
  return cluster_->root();
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

ExplorerServerItem* ExplorerDatabaseItem::parent() const {
  return dynamic_cast<ExplorerServerItem*>(IExplorerTreeItem::parent());
}

QString ExplorerDatabaseItem::name() const {
  return common::convertFromString<QString>(db_->name());
}

ExplorerDatabaseItem::eType ExplorerDatabaseItem::type() const {
  return eDatabase;
}

bool ExplorerDatabaseItem::isDefault() const {
    return info()->isDefault();
}

size_t ExplorerDatabaseItem::sizeDB() const {
  core::IDataBaseInfoSPtr inf = info();
  return inf->sizeDB();
}

size_t ExplorerDatabaseItem::loadedSize() const {
  core::IDataBaseInfoSPtr inf = info();
  return inf->loadedSize();
}

core::IServerSPtr ExplorerDatabaseItem::server() const {
  ExplorerServerItem* serv = dynamic_cast<ExplorerServerItem*>(parent_);
  if (!serv) {
    return core::IServerSPtr();
  }

  return serv->server();
}

core::IDatabaseSPtr ExplorerDatabaseItem::db() const {
  return db_;
}

void ExplorerDatabaseItem::loadContent(const std::string& pattern, uint32_t countKeys) {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::events_info::LoadDatabaseContentRequest req(this, dbs->info(), pattern, countKeys);
    dbs->loadContent(req);
  }
}

void ExplorerDatabaseItem::setDefault() {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::events_info::SetDefaultDatabaseRequest req(this, dbs->info());
    dbs->setDefault(req);
  }
}

core::IDataBaseInfoSPtr ExplorerDatabaseItem::info() const {
  return db_->info();
}

void ExplorerDatabaseItem::removeKey(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::CommandKeySPtr cmd(new core::CommandDeleteKey(key));
    core::events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::loadValue(const core::NDbKValue& key) {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::CommandKeySPtr cmd(new core::CommandLoadKey(key));
    core::events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::createKey(const core::NDbKValue &key) {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::CommandKeySPtr cmd(new core::CommandCreateKey(key));
    core::events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::removeAllKeys() {
  core::IDatabaseSPtr dbs = db();
  if (dbs) {
    core::events_info::ClearDatabaseRequest req(this, dbs->info());
    dbs->removeAllKeys(req);
  }
}

ExplorerKeyItem::ExplorerKeyItem(const core::NDbKValue& key, ExplorerDatabaseItem* parent)
    : IExplorerTreeItem(parent), key_(key) {
}

ExplorerDatabaseItem* ExplorerKeyItem::parent() const {
  return dynamic_cast<ExplorerDatabaseItem*>(parent_);
}

core::NDbKValue ExplorerKeyItem::key() const {
  return key_;
}

QString ExplorerKeyItem::name() const {
  return common::convertFromString<QString>(key_.keyString());
}

core::IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(parent_);
  if (!db) {
    return core::IServerSPtr();
  }

  return db->server();
}

IExplorerTreeItem::eType ExplorerKeyItem::type() const {
  return eKey;
}

void ExplorerKeyItem::removeFromDb() {
  ExplorerDatabaseItem* par = parent();
  if (par) {
    par->removeKey(key_);
  }
}

void ExplorerKeyItem::loadValueFromDb() {
  ExplorerDatabaseItem* par = parent();
  if (par) {
    par->loadValue(key_);
  }
}

ExplorerTreeModel::ExplorerTreeModel(QObject* parent)
  : TreeModel(parent) {
}

QVariant ExplorerTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  IExplorerTreeItem* node = common::utils_qt::item<IExplorerTreeItem*>(index);

  if (!node) {
    return QVariant();
  }

  int col = index.column();
  IExplorerTreeItem::eType t = node->type();

  if (role == Qt::ToolTipRole) {
    core::IServerSPtr server = node->server();
    if (t == IExplorerTreeItem::eServer && server) {
      core::ServerDiscoveryInfoSPtr disc = server->discoveryInfo();
      if (disc) {
        QString dname = common::convertFromString<QString>(disc->name());
        QString dtype = common::convertFromString<QString>(common::convertToString(disc->type()));
        QString dhost = common::convertFromString<QString>(common::convertToString(disc->host()));
        return QString("<b>Name:</b> %1<br/>"
                       "<b>Type:</b> %2<br/>"
                       "<b>Host:</b> %3<br/>").arg(dname).arg(dtype).arg(dhost);
      } else {
        QString sname = common::convertFromString<QString>(server->name());
        bool isCanRemote = server->isCanRemote();
        if (isCanRemote) {
          core::IServerRemote* rserver = dynamic_cast<core::IServerRemote*>(server.get());
          CHECK(rserver);
          QString stype = common::convertFromString<QString>(common::convertToString(rserver->role()));
          QString shost = common::convertFromString<QString>(common::convertToString(rserver->host()));
          return QString("<b>Name:</b> %1<br/>"
                         "<b>Type:</b> %2<br/>"
                         "<b>Host:</b> %3<br/>").arg(sname).arg(stype).arg(shost);
        } else {
          core::IServerLocal* lserver = dynamic_cast<core::IServerLocal*>(server.get());
          CHECK(lserver);
          QString spath = common::convertFromString<QString>(lserver->path());
          return QString("<b>Name:</b> %1<br/>"
                         "<b>Path:</b> %3<br/>").arg(sname).arg(spath);
        }
      }
    } else if (t == IExplorerTreeItem::eDatabase) {
        ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(node);
        if (db && db->isDefault()) {
          return QString("<b>Db size:</b> %1 keys<br/>").arg(db->sizeDB());
        }
    }
  }

  if (role == Qt::DecorationRole && col == ExplorerServerItem::eName) {
    if (t == IExplorerTreeItem::eCluster) {
      return GuiFactory::instance().clusterIcon();
    } else if (t == IExplorerTreeItem::eServer) {
      return GuiFactory::instance().icon(node->server()->type());
    } else if (t == IExplorerTreeItem::eKey) {
      return GuiFactory::instance().keyIcon();
    } else if (t == IExplorerTreeItem::eDatabase) {
      return GuiFactory::instance().databaseIcon();
    } else {
      NOTREACHED();
    }
  }

  if (role == Qt::DisplayRole) {
    if (col == IExplorerTreeItem::eName) {
      if (t == IExplorerTreeItem::eKey) {
        return node->name();
      } else if (t == IExplorerTreeItem::eDatabase) {
        ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(node);
        if (db) {
          return QString("%1 (%2/%3)").arg(node->name()).arg(db->childrenCount()).arg(db->sizeDB());
        }
      } else {
        return QString("%1 (%2)").arg(node->name()).arg(node->childrenCount());
      }
    }
  }

  if (role == Qt::ForegroundRole) {
    if (t == IExplorerTreeItem::eDatabase) {
      ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(node);
      if (db && db->isDefault()) {
        return QVariant( QColor( Qt::red ) );
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags ExplorerTreeModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags result = 0;
  if (index.isValid()) {
    result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
  return result;
}

QVariant ExplorerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == ExplorerServerItem::eName) {
      return translations::trName;
    }
  }

  return TreeModel::headerData(section, orientation, role);
}

int ExplorerTreeModel::columnCount(const QModelIndex& parent) const {
  return ExplorerServerItem::eCountColumns;
}

void ExplorerTreeModel::addCluster(core::IClusterSPtr cluster) {
  ExplorerClusterItem* cl = findClusterItem(cluster);
  if (!cl) {
    fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
    if (!parent) {
      DNOTREACHED();
      return;
    }

    ExplorerClusterItem* item = new ExplorerClusterItem(cluster, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeCluster(core::IClusterSPtr cluster) {
  fasto::qt::gui::TreeItem* par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  if (!par) {
    NOTREACHED();
    return;
  }

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
    fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
    if (!parent) {
      DNOTREACHED();
      return;
    }

    ExplorerServerItem* item = new ExplorerServerItem(server, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeServer(core::IServerSPtr server) {
  fasto::qt::gui::TreeItem* par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  if (!par) {
    DNOTREACHED();
    return;
  }

  ExplorerServerItem* serverItem = findServerItem(server.get());
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addDatabase(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    DNOTREACHED();
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    QModelIndex ind = index(root_->indexOf(parent), 0, QModelIndex());
    ExplorerDatabaseItem* item = new ExplorerDatabaseItem(server->createDatabaseByInfo(db), parent);
    insertItem(ind, item);
  }
}

void ExplorerTreeModel::removeDatabase(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    DNOTREACHED();
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (dbs) {
    QModelIndex ind = index(root_->indexOf(parent), 0, QModelIndex());
    removeItem(ind, dbs);
  }
}

void ExplorerTreeModel::setDefaultDb(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    DNOTREACHED();
    return;
  }

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
  if (!parent) {
    DNOTREACHED();
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    DNOTREACHED();
    return;
  }

  int index_db = parent->indexOf(dbs);
  QModelIndex dbs_index1 = createIndex(index_db, ExplorerDatabaseItem::eName, dbs);
  QModelIndex dbs_index2 = createIndex(index_db, ExplorerDatabaseItem::eCountColumns, dbs);
  updateItem(dbs_index1, dbs_index2);
}

void ExplorerTreeModel::addKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue &dbv) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, dbv);
  if (!keyit) {
    QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
    ExplorerKeyItem* item = new ExplorerKeyItem(dbv, dbs);
    insertItem(parentdb, item);
  }
}

void ExplorerTreeModel::removeKey(core::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue &key) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, key);
  if (keyit) {
    QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
    removeItem(parentdb, keyit);
  }
}

void ExplorerTreeModel::removeAllKeys(core::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    return;
  }

  QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
  removeAllItems(parentdb);
}

ExplorerClusterItem* ExplorerTreeModel::findClusterItem(core::IClusterSPtr cl) {
  fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  if (!parent) {
    DNOTREACHED();
    return nullptr;
  }

  for (size_t i = 0; i < parent->childrenCount() ; ++i) {
    ExplorerClusterItem* item = dynamic_cast<ExplorerClusterItem*>(parent->child(i));
    if (item && item->cluster() == cl) {
      return item;
    }
  }
  return nullptr;
}

ExplorerServerItem* ExplorerTreeModel::findServerItem(core::IServer* server) const {
  fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  if (!parent) {
    DNOTREACHED();
    return nullptr;
  }

  for (size_t i = 0; i < parent->childrenCount(); ++i) {
    ExplorerServerItem* item = dynamic_cast<ExplorerServerItem*>(parent->child(i));
    if (item) {
      if (item->server().get() == server) {
        return item;
      }
    } else {
      ExplorerClusterItem* citem = dynamic_cast<ExplorerClusterItem*>(parent->child(i));
      if (citem) {
        for (size_t j = 0; j < citem->childrenCount(); ++j) {
          ExplorerServerItem* item = dynamic_cast<ExplorerServerItem*>(citem->child(i));
           if (item) {
             if (item->server().get() == server) {
               return item;
             }
           }
         }
       }
    }
  }
  return nullptr;
}

ExplorerDatabaseItem* ExplorerTreeModel::findDatabaseItem(ExplorerServerItem* server,
                                                          core::IDataBaseInfoSPtr db) const {
  if (!server) {
    return nullptr;
  }

  for (size_t i = 0; i < server->childrenCount() ; ++i) {
    ExplorerDatabaseItem* item = dynamic_cast<ExplorerDatabaseItem*>(server->child(i));
    if (!item) {
      DNOTREACHED();
      continue;
    }

    core::IDatabaseSPtr inf = item->db();
    if (inf && inf->name() == db->name()) {
      return item;
    }
  }

  return nullptr;
}

ExplorerKeyItem* ExplorerTreeModel::findKeyItem(ExplorerDatabaseItem* db,
                                                const core::NDbKValue &key) const {
  if (!db) {
    return nullptr;
  }

  for (size_t i = 0; i < db->childrenCount() ; ++i) {
    ExplorerKeyItem* item = dynamic_cast<ExplorerKeyItem*>(db->child(i));
    if (!item) {
      DNOTREACHED();
      continue;
    }

    if (item->key().keyString() == key.keyString()) {
      return item;
    }
  }

  return nullptr;
}

}  // namespace gui
}  // namespace fastonosql
