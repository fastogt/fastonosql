/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "gui/models/explorer_tree_model.h"

#include <string>

#include <QIcon>

#include <common/net/types.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "proxy/server/iserver_local.h"
#include "proxy/server/iserver_remote.h"

#include "gui/gui_factory.h"
#include "gui/key_info.h"
#include "gui/models/items/explorer_tree_item.h"

#include "translations/global.h"

namespace {
const QString trDiscoveryToolTipTemplate_3S = QObject::tr(
    "<b>Name:</b> %1<br/>"
    "<b>Type:</b> %2<br/>"
    "<b>Host:</b> %3<br/>");
const QString trRemoteServerToolTipTemplate_4S = QObject::tr(
    "<b>Name:</b> %1<br/>"
    "<b>Type:</b> %2<br/>"
    "<b>Mode:</b> %3<br/>"
    "<b>Host:</b> %4<br/>");
const QString trLocalServerToolTipTemplate_2S = QObject::tr(
    "<b>Name:</b> %1<br/>"
    "<b>Path:</b> %3<br/>");
const QString trDbToolTipTemplate_1S = QObject::tr("<b>Db size:</b> %1 keys<br/>");
const QString trNamespace_1S = QObject::tr("<b>Group size:</b> %1 keys<br/>");
const QString trKey_1S = QObject::tr("Key displayed in: <b>%1</b> format<br/>");
}  // namespace

namespace fastonosql {
namespace gui {

ExplorerTreeModel::ExplorerTreeModel(QObject* parent) : TreeModel(parent) {}

QVariant ExplorerTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(index);
  if (!node) {
    NOTREACHED();
    return QVariant();
  }

  int col = index.column();
  IExplorerTreeItem::eType type = node->type();

  if (role == Qt::ToolTipRole) {
    if (type == IExplorerTreeItem::eServer) {
      ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);
      proxy::IServerSPtr server = server_node->server();
      QString sname;
      common::ConvertFromString(server->GetName(), &sname);
      bool is_can_remote = server->IsCanRemote();
      if (is_can_remote) {
        proxy::IServerRemote* rserver = static_cast<proxy::IServerRemote*>(server.get());
        QString stype;
        common::ConvertFromString(common::ConvertToString(rserver->GetRole()), &stype);
        QString mtype;
        common::ConvertFromString(common::ConvertToString(rserver->GetMode()), &mtype);
        QString shost = translations::trCalculate + "...";
        common::ConvertFromString(common::ConvertToString(rserver->GetHost()), &shost);
        return trRemoteServerToolTipTemplate_4S.arg(sname, stype, mtype, shost);
      } else {
        proxy::IServerLocal* lserver = static_cast<proxy::IServerLocal*>(server.get());
        QString spath = translations::trCalculate + "...";
        common::ConvertFromString(lserver->GetPath(), &spath);
        return trLocalServerToolTipTemplate_2S.arg(sname, spath);
      }
    } else if (type == IExplorerTreeItem::eDatabase) {
      ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);
      if (db->isDefault()) {
        return trDbToolTipTemplate_1S.arg(db->totalKeysCount());
      }
    } else if (type == IExplorerTreeItem::eNamespace) {
      ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);
      return trNamespace_1S.arg(ns->keysCount());
    } else if (type == IExplorerTreeItem::eKey) {
      ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);
      const core::NKey nkey = key->key();
      const auto key_str = nkey.GetKey();
      return trKey_1S.arg(key_str.GetType() == core::nkey_t::BINARY_DATA ? "hex" : "text");
    }

    return QVariant();
  }

  if (role == Qt::DecorationRole && col == eName) {
    if (type == IExplorerTreeItem::eServer) {
      ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);
      proxy::IServerSPtr server = server_node->server();
      return GuiFactory::GetInstance().icon(server->GetType());
    } else if (type == IExplorerTreeItem::eKey) {
      ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);
      core::NKey nkey = key->key();
      if (nkey.GetTTL() == NO_TTL) {
        return GuiFactory::GetInstance().keyIcon();
      }
      return GuiFactory::GetInstance().keyTTLIcon();
    } else if (type == IExplorerTreeItem::eDatabase) {
      return GuiFactory::GetInstance().databaseIcon();
    } else if (type == IExplorerTreeItem::eNamespace) {
      return GuiFactory::GetInstance().directoryIcon();
    }
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
    else if (type == IExplorerTreeItem::eCluster) {
      return GuiFactory::GetInstance().clusterIcon();
    } else if (type == IExplorerTreeItem::eSentinel) {
      return GuiFactory::GetInstance().sentinelIcon();
    }
#endif
    else {
      NOTREACHED();
    }
  }

  if (role == Qt::DisplayRole) {
    if (col == eName) {
      if (type == IExplorerTreeItem::eKey) {
        return node->name();
      } else if (type == IExplorerTreeItem::eDatabase) {
        ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);
        return QString("%1 (%2/%3)").arg(node->name()).arg(db->loadedKeysCount()).arg(db->totalKeysCount());  // db
      } else if (type == IExplorerTreeItem::eNamespace) {
        ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);
        return QString("%1 (%2)").arg(node->name()).arg(ns->keysCount());  // db
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
    } else if (type == IExplorerTreeItem::eKey) {
      ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);
      const core::NKey nkey = key->key();
      const auto key_str = nkey.GetKey();
      if (key_str.GetType() == core::nkey_t::BINARY_DATA) {
        return QVariant(QColor(Qt::gray));
      }
    }
  }

  return QVariant();
}  // namespace gui

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

  if (orientation == Qt::Horizontal) {
    if (section == eName) {
      return translations::trName;
    }
  }

  return TreeModel::headerData(section, orientation, role);
}

int ExplorerTreeModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return eCountColumns;
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
void ExplorerTreeModel::addCluster(proxy::IClusterSPtr cluster) {
  if (!cluster) {
    return;
  }

  ExplorerClusterItem* cl = findClusterItem(cluster);
  if (!cl) {
    common::qt::gui::TreeItem* parent = root();
    CHECK(parent);

    ExplorerClusterItem* item = new ExplorerClusterItem(cluster, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeCluster(proxy::IClusterSPtr cluster) {
  if (!cluster) {
    return;
  }

  ExplorerClusterItem* serverItem = findClusterItem(cluster);
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}
#endif

void ExplorerTreeModel::addServer(proxy::IServerSPtr server) {
  if (!server) {
    return;
  }

  ExplorerServerItem* serv = findServerItem(server.get());
  if (!serv) {
    common::qt::gui::TreeItem* parent = root();
    CHECK(parent);

    ExplorerServerItem* item = new ExplorerServerItem(server, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeServer(proxy::IServerSPtr server) {
  if (!server) {
    return;
  }

  ExplorerServerItem* serverItem = findServerItem(server.get());
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
void ExplorerTreeModel::addSentinel(proxy::ISentinelSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  ExplorerSentinelItem* cl = findSentinelItem(sentinel);
  if (!cl) {
    common::qt::gui::TreeItem* parent = root();
    CHECK(parent);

    ExplorerSentinelItem* item = new ExplorerSentinelItem(sentinel, parent);
    insertItem(QModelIndex(), item);
  }
}

void ExplorerTreeModel::removeSentinel(proxy::ISentinelSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  ExplorerSentinelItem* serverItem = findSentinelItem(sentinel);
  if (serverItem) {
    removeItem(QModelIndex(), serverItem);
  }
}
#endif

void ExplorerTreeModel::addDatabase(proxy::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    common::qt::gui::TreeItem* parent_server = parent->parent();
    QModelIndex parent_index = createIndex(parent_server->indexOf(parent), 0, parent);
    ExplorerDatabaseItem* item = new ExplorerDatabaseItem(server->CreateDatabaseByInfo(db), parent);
    insertItem(parent_index, item);
  }
}

void ExplorerTreeModel::removeDatabase(proxy::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (dbs) {
    common::qt::gui::TreeItem* parent_server = parent->parent();
    QModelIndex index = createIndex(parent_server->indexOf(parent), 0, dbs);
    removeItem(index.parent(), dbs);
  }
}

void ExplorerTreeModel::setDefaultDb(proxy::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    DNOTREACHED();
    return;
  }

  QModelIndex dbs_first_index = createIndex(0, eName, parent);
  QModelIndex dbs_last_index = createIndex(parent->childrenCount() - 1, eCountColumns - 1, parent);
  updateItem(dbs_first_index, dbs_last_index);
}

void ExplorerTreeModel::updateDb(proxy::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  QModelIndex dbs_index1 = createIndex(db_index, eName, dbs);
  QModelIndex dbs_index2 = createIndex(db_index, eCountColumns - 1, dbs);
  updateItem(dbs_index1, dbs_index2);
}

void ExplorerTreeModel::addKey(proxy::IServer* server,
                               core::IDataBaseInfoSPtr db,
                               const core::NDbKValue& dbv,
                               const std::string& ns_separator,
                               proxy::NsDisplayStrategy ns_strategy) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  findOrCreateKey(dbs, dbv, ns_separator, ns_strategy);
}

void ExplorerTreeModel::removeKey(proxy::IServer* server, core::IDataBaseInfoSPtr db, const core::NKey& key) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, key);
  if (keyit) {
    common::qt::gui::TreeItem* par = keyit->parent();
    QModelIndex index = createIndex(par->indexOf(keyit), 0, keyit);
    removeItem(index.parent(), keyit);

    IExplorerTreeItem* node = static_cast<IExplorerTreeItem*>(par);
    if (node->type() == IExplorerTreeItem::eNamespace) {
      ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);
      if (ns->childrenCount() == 0) {
        auto gpa = ns->parent();
        const int pos = gpa->indexOf(ns);
        QModelIndex dindex = createIndex(pos, 0, ns);
        removeItem(dindex.parent(), ns);
      }
    }
  }
}

void ExplorerTreeModel::renameKey(proxy::IServer* server,
                                  core::IDataBaseInfoSPtr db,
                                  const core::NKey& old_key,
                                  const core::NKey& new_key,
                                  const std::string& ns_separator,
                                  proxy::NsDisplayStrategy ns_strategy) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, old_key);
  core::NDbKValue dbv;
  if (keyit) {
    dbv = keyit->dbv();
    common::qt::gui::TreeItem* par = keyit->parent();
    QModelIndex index = createIndex(par->indexOf(keyit), 0, keyit);
    removeItem(index.parent(), keyit);
  }

  dbv.SetKey(new_key);
  createKey(dbs, dbv, ns_separator, ns_strategy);
}

void ExplorerTreeModel::updateKey(proxy::IServer* server,
                                  core::IDataBaseInfoSPtr db,
                                  const core::NKey& old_key,
                                  const core::NKey& new_key) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, old_key);
  if (keyit) {
    common::qt::gui::TreeItem* par = keyit->parent();
    int index_key = par->indexOf(keyit);
    keyit->setKey(new_key);
    QModelIndex key_index1 = createIndex(index_key, eName, dbs);
    QModelIndex key_index2 = createIndex(index_key, eCountColumns - 1, dbs);
    updateItem(key_index1, key_index2);
  }
}

void ExplorerTreeModel::updateValue(proxy::IServer* server, core::IDataBaseInfoSPtr db, const core::NDbKValue& dbv) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  ExplorerKeyItem* keyit = findKeyItem(dbs, dbv.GetKey());
  if (keyit) {
    common::qt::gui::TreeItem* par = keyit->parent();
    int index_key = par->indexOf(keyit);
    keyit->setDbv(dbv);
    QModelIndex key_index1 = createIndex(index_key, eName, dbs);
    QModelIndex key_index2 = createIndex(index_key, eCountColumns - 1, dbs);
    updateItem(key_index1, key_index2);
  }
}

void ExplorerTreeModel::removeAllKeys(proxy::IServer* server, core::IDataBaseInfoSPtr db) {
  ExplorerServerItem* parent = findServerItem(server);
  if (!parent) {
    return;
  }

  int db_index = 0;
  ExplorerDatabaseItem* dbs = findDatabaseItem(parent, db, &db_index);
  if (!dbs) {
    return;
  }

  QModelIndex parentdb = createIndex(db_index, eName, dbs);
  removeAllItems(parentdb);
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
ExplorerClusterItem* ExplorerTreeModel::findClusterItem(proxy::IClusterSPtr cl) {
  common::qt::gui::TreeItem* parent = root();
  if (!parent) {
    return nullptr;
  }

  for (size_t i = 0; i < parent->childrenCount(); ++i) {
    ExplorerClusterItem* cluster_item = static_cast<ExplorerClusterItem*>(parent->child(i));
    if (!cluster_item || cluster_item->type() != IExplorerTreeItem::eCluster) {
      continue;
    }

    if (cluster_item && cluster_item->cluster() == cl) {
      return cluster_item;
    }
  }
  return nullptr;
}

ExplorerSentinelItem* ExplorerTreeModel::findSentinelItem(proxy::ISentinelSPtr sentinel) {
  common::qt::gui::TreeItem* parent = root();
  if (!parent) {
    return nullptr;
  }

  for (size_t i = 0; i < parent->childrenCount(); ++i) {
    ExplorerSentinelItem* sentinel_item = static_cast<ExplorerSentinelItem*>(parent->child(i));
    if (!sentinel_item || sentinel_item->type() != IExplorerTreeItem::eSentinel) {
      continue;
    }

    if (sentinel_item && sentinel_item->sentinel() == sentinel) {
      return sentinel_item;
    }
  }
  return nullptr;
}
#endif

ExplorerServerItem* ExplorerTreeModel::findServerItem(proxy::IServer* server) const {
  return static_cast<ExplorerServerItem*>(
      common::qt::gui::findItemRecursive(root(), [server](common::qt::gui::TreeItem* item) -> bool {
        IExplorerTreeItem* exp_item = static_cast<IExplorerTreeItem*>(item);
        if (exp_item->type() != IExplorerTreeItem::eServer) {
          return false;
        }

        return static_cast<ExplorerServerItem*>(exp_item)->server().get() == server;
      }));
}

ExplorerDatabaseItem* ExplorerTreeModel::findDatabaseItem(ExplorerServerItem* server,
                                                          core::IDataBaseInfoSPtr db,
                                                          int* index) const {
  if (!server || !index) {
    DNOTREACHED();
    return nullptr;
  }

  for (size_t i = 0; i < server->childrenCount(); ++i) {
    ExplorerDatabaseItem* db_item = static_cast<ExplorerDatabaseItem*>(server->child(i));
    if (db_item->type() != IExplorerTreeItem::eDatabase) {
      continue;
    }

    proxy::IDatabaseSPtr inf = db_item->db();
    if (inf && inf->GetName() == db->GetName()) {
      *index = i;
      return db_item;
    }
  }

  return nullptr;
}

ExplorerKeyItem* ExplorerTreeModel::findKeyItem(IExplorerTreeItem* db_or_ns, const core::NKey& key) const {
  return static_cast<ExplorerKeyItem*>(
      common::qt::gui::findItemRecursive(db_or_ns, [&key](common::qt::gui::TreeItem* item) -> bool {
        ExplorerKeyItem* key_item = static_cast<ExplorerKeyItem*>(item);
        if (key_item->type() != IExplorerTreeItem::eKey) {
          return false;
        }

        return key_item->equalsKey(key);
      }));
}

ExplorerNSItem* ExplorerTreeModel::findOrCreateNSItem(IExplorerTreeItem* db_or_ns,
                                                      const std::vector<core::readable_string_t>& namespaces,
                                                      const std::string& separator) {
  IExplorerTreeItem* par = db_or_ns;
  ExplorerNSItem* founded_item = nullptr;
  for (size_t i = 0; i < namespaces.size(); ++i) {
    ExplorerNSItem* item = nullptr;
    const auto cur_ns = namespaces[i];

    for (size_t j = 0; j < par->childrenCount(); ++j) {
      ExplorerNSItem* ns_item = static_cast<ExplorerNSItem*>(par->child(j));
      if (ns_item->type() != IExplorerTreeItem::eNamespace) {
        continue;
      }

      if (ns_item->basicStringName() == cur_ns) {
        item = ns_item;
      }
    }

    if (!item) {
      common::qt::gui::TreeItem* gpar = par->parent();
      QModelIndex parentdb = createIndex(gpar->indexOf(par), eName, par);
      item = new ExplorerNSItem(cur_ns, separator, par);
      insertItem(parentdb, item);
    }

    par = item;
    founded_item = item;
  }

  CHECK(founded_item);
  return founded_item;
}

ExplorerKeyItem* ExplorerTreeModel::findOrCreateKey(ExplorerDatabaseItem* dbs,
                                                    const core::NDbKValue& dbv,
                                                    const std::string& separator,
                                                    proxy::NsDisplayStrategy strategy) {
  const core::NKey key = dbv.GetKey();
  ExplorerKeyItem* keyit = findKeyItem(dbs, key);
  if (keyit) {
    return keyit;
  }

  return createKey(dbs, dbv, separator, strategy);
}

ExplorerKeyItem* ExplorerTreeModel::createKey(ExplorerDatabaseItem* dbs,
                                              const core::NDbKValue& dbv,
                                              const std::string& separator,
                                              proxy::NsDisplayStrategy strategy) {
  const core::NKey key = dbv.GetKey();
  IExplorerTreeItem* nitem = dbs;
  const auto key_str = key.GetKey();
  KeyInfo kinf(key_str.GetHumanReadable(), separator);
  if (kinf.hasNamespace()) {
    nitem = findOrCreateNSItem(dbs, kinf.namespaces(), kinf.nsSeparator());
  }

  common::qt::gui::TreeItem* parent_nitem = nitem->parent();
  QModelIndex parent_index = createIndex(parent_nitem->indexOf(nitem), eName, nitem);
  ExplorerKeyItem* item = new ExplorerKeyItem(dbv, separator, strategy, nitem);
  insertItem(parent_index, item);
  updateItem(parent_index, parent_index);  // refresh counters
  return item;
}

}  // namespace gui
}  // namespace fastonosql
