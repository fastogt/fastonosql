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

IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent)
  : TreeItem(parent) {
}

ExplorerServerItem::ExplorerServerItem(IServerSPtr server, TreeItem* parent)
  : IExplorerTreeItem(parent), server_(server) {
}

QString ExplorerServerItem::name() const {
  return server_->name();
}

IServerSPtr ExplorerServerItem::server() const {
  return server_;
}

ExplorerServerItem::eType ExplorerServerItem::type() const {
  return eServer;
}

void ExplorerServerItem::loadDatabases() {
  events_info::LoadDatabasesInfoRequest req(this);
  return server_->loadDatabases(req);
}

ExplorerClusterItem::ExplorerClusterItem(IClusterSPtr cluster, TreeItem* parent)
  : IExplorerTreeItem(parent), cluster_(cluster) {
  ICluster::nodes_type nodes = cluster_->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
    addChildren(ser);
  }
}

QString ExplorerClusterItem::name() const {
  return cluster_->name();
}

IServerSPtr ExplorerClusterItem::server() const {
  return cluster_->root();
}

ExplorerClusterItem::eType ExplorerClusterItem::type() const {
  return eCluster;
}

IClusterSPtr ExplorerClusterItem::cluster() const {
  return cluster_;
}

ExplorerDatabaseItem::ExplorerDatabaseItem(IDatabaseSPtr db, ExplorerServerItem* parent)
  : IExplorerTreeItem(parent), db_(db) {
  DCHECK(db);
}

ExplorerServerItem *ExplorerDatabaseItem::parent() const {
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
  IDataBaseInfoSPtr inf = info();
  return inf->sizeDB();
}

size_t ExplorerDatabaseItem::loadedSize() const {
  IDataBaseInfoSPtr inf = info();
  return inf->loadedSize();
}

IServerSPtr ExplorerDatabaseItem::server() const {
  ExplorerServerItem* serv = dynamic_cast<ExplorerServerItem*>(parent_);
  if (!serv) {
    return IServerSPtr();
  }

  return serv->server();
}

IDatabaseSPtr ExplorerDatabaseItem::db() const {
  return db_;
}

void ExplorerDatabaseItem::loadContent(const std::string& pattern, uint32_t countKeys) {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    events_info::LoadDatabaseContentRequest req(this, dbs->info(), pattern, countKeys);
    dbs->loadContent(req);
  }
}

void ExplorerDatabaseItem::setDefault() {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    events_info::SetDefaultDatabaseRequest req(this, dbs->info());
    dbs->setDefault(req);
  }
}

IDataBaseInfoSPtr ExplorerDatabaseItem::info() const {
  return db_->info();
}

void ExplorerDatabaseItem::removeKey(const NDbKValue& key) {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    CommandKeySPtr cmd(new CommandDeleteKey(key));
    events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::loadValue(const NDbKValue& key) {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    CommandKeySPtr cmd(new CommandLoadKey(key));
    events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::createKey(const NDbKValue &key) {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    CommandKeySPtr cmd(new CommandCreateKey(key));
    events_info::CommandRequest req(this, dbs->info(), cmd);
    dbs->executeCommand(req);
  }
}

void ExplorerDatabaseItem::removeAllKeys() {
  IDatabaseSPtr dbs = db();
  if (dbs) {
    events_info::ClearDatabaseRequest req(this, dbs->info());
    dbs->removeAllKeys(req);
  }
}

ExplorerKeyItem::ExplorerKeyItem(const NDbKValue& key, ExplorerDatabaseItem* parent)
    : IExplorerTreeItem(parent), key_(key) {
}

ExplorerDatabaseItem* ExplorerKeyItem::parent() const {
  return dynamic_cast<ExplorerDatabaseItem*>(parent_);
}

NDbKValue ExplorerKeyItem::key() const {
  return key_;
}

QString ExplorerKeyItem::name() const {
  return common::convertFromString<QString>(key_.keyString());
}

IServerSPtr ExplorerKeyItem::server() const {
  ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(parent_);
  if (!db) {
    return IServerSPtr();
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

ExplorerTreeModel::ExplorerTreeModel(QObject *parent)
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
    IServerSPtr server = node->server();
    if (t == IExplorerTreeItem::eServer && server) {
      ServerDiscoveryInfoSPtr disc = server->discoveryInfo();
      if (disc) {
        QString dname = common::convertFromString<QString>(disc->name());
        QString dtype = common::convertFromString<QString>(common::convertToString(disc->type()));
        QString dhost = common::convertFromString<QString>(common::convertToString(disc->host()));
        return QString("<b>Name:</b> %1<br/>"
                       "<b>Type:</b> %2<br/>"
                       "<b>Host:</b> %3<br/>").arg(dname).arg(dtype).arg(dhost);
      } else {
        QString sname = server->name();
        bool isCanRemote = server->isCanRemote();
        if (isCanRemote) {
          IServerRemote* rserver = dynamic_cast<IServerRemote*>(server.get());
          CHECK(rserver);
          QString stype = common::convertFromString<QString>(common::convertToString(rserver->role()));
          QString shost = common::convertFromString<QString>(common::convertToString(rserver->host()));
          return QString("<b>Name:</b> %1<br/>"
                         "<b>Type:</b> %2<br/>"
                         "<b>Host:</b> %3<br/>").arg(sname).arg(stype).arg(shost);
        } else {
          IServerLocal* lserver = dynamic_cast<IServerLocal*>(server.get());
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

void ExplorerTreeModel::addCluster(IClusterSPtr cluster) {
  ExplorerClusterItem* cl = findClusterItem(cluster);
  if (!cl) {
    fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
    DCHECK(parent);
    if (!parent) {
      return;
    }

    ExplorerClusterItem* item = new ExplorerClusterItem(cluster, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeCluster(IClusterSPtr cluster) {
  fasto::qt::gui::TreeItem* par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  DCHECK(par);
  if (!par) {
    return;
  }

  ExplorerClusterItem* serverItem = findClusterItem(cluster);
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addServer(IServerSPtr server) {
  if (!server) {
    return;
  }

  ExplorerServerItem* serv = findServerItem(server.get());
  if (!serv) {
    fasto::qt::gui::TreeItem *parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
    DCHECK(parent);
    if (!parent) {
      return;
    }

    ExplorerServerItem* item = new ExplorerServerItem(server, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeServer(IServerSPtr server) {
  fasto::qt::gui::TreeItem* par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  DCHECK(par);
  if (!par) {
    return;
  }

  ExplorerServerItem* serverItem = findServerItem(server.get());
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

void ExplorerTreeModel::addDatabase(IServer* server, IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  DCHECK(parent);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (!dbs) {
    QModelIndex ind = index(root_->indexOf(parent), 0, QModelIndex());
    ExplorerDatabaseItem *item = new ExplorerDatabaseItem(server->createDatabaseByInfo(db), parent);
    insertItem(ind, item);
  }
}

void ExplorerTreeModel::removeDatabase(IServer* server, IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  DCHECK(parent);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  if (dbs) {
    QModelIndex ind = index(root_->indexOf(parent), 0, QModelIndex());
    removeItem(ind, dbs);
  }
}

void ExplorerTreeModel::setDefaultDb(IServer* server, IDataBaseInfoSPtr db) {
  ExplorerServerItem *parent = findServerItem(server);
  DCHECK(parent);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  DCHECK(dbs);
  if (!dbs) {
    return;
  }

  QModelIndex parent_index = createIndex(root_->indexOf(parent), ExplorerDatabaseItem::eName, parent);
  QModelIndex dbs_last_index = index(parent->childrenCount(), ExplorerDatabaseItem::eCountColumns, parent_index);
  updateItem(parent_index, dbs_last_index);
}

void ExplorerTreeModel::updateDb(IServer* server, IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  DCHECK(parent);
  if (!parent) {
    return;
  }

  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db);
  DCHECK(dbs);
  if (!dbs) {
    return;
  }

  int index_db = parent->indexOf(dbs);
  QModelIndex dbs_index1 = createIndex(index_db, ExplorerDatabaseItem::eName, dbs);
  QModelIndex dbs_index2 = createIndex(index_db, ExplorerDatabaseItem::eCountColumns, dbs);
  updateItem(dbs_index1, dbs_index2);
}

void ExplorerTreeModel::addKey(IServer* server, IDataBaseInfoSPtr db, const NDbKValue &dbv) {
  ExplorerServerItem *parent = findServerItem(server);
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
    ExplorerKeyItem *item = new ExplorerKeyItem(dbv, dbs);
    insertItem(parentdb, item);
  }
}

void ExplorerTreeModel::removeKey(IServer* server, IDataBaseInfoSPtr db, const NDbKValue &key) {
  ExplorerServerItem *parent = findServerItem(server);
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

void ExplorerTreeModel::removeAllKeys(IServer* server, IDataBaseInfoSPtr db) {
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

ExplorerClusterItem* ExplorerTreeModel::findClusterItem(IClusterSPtr cl) {
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

ExplorerServerItem* ExplorerTreeModel::findServerItem(IServer* server) const {
  fasto::qt::gui::TreeItem* parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
  DCHECK(parent);
  if (!parent) {
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

ExplorerDatabaseItem *ExplorerTreeModel::findDatabaseItem(ExplorerServerItem* server,
                                                          IDataBaseInfoSPtr db) const {
  if (!server) {
    return nullptr;
  }

  for (size_t i = 0; i < server->childrenCount() ; ++i) {
    ExplorerDatabaseItem* item = dynamic_cast<ExplorerDatabaseItem*>(server->child(i));
    DCHECK(item);
    if (!item) {
      continue;
    }

    IDatabaseSPtr inf = item->db();
    if (inf && inf->name() == db->name()) {
      return item;
    }
  }

  return nullptr;
}

ExplorerKeyItem *ExplorerTreeModel::findKeyItem(ExplorerDatabaseItem* db,
                                                const NDbKValue &key) const {
  if (!db) {
    return nullptr;
  }

  for (size_t i = 0; i < db->childrenCount() ; ++i) {
    ExplorerKeyItem* item = dynamic_cast<ExplorerKeyItem*>(db->child(i));
    DCHECK(item);
    if (!item) {
      continue;
    }

    if (item->key().keyString() == key.keyString()) {
      return item;
    }
  }

  return nullptr;
}

}  // namespace fastonosql
