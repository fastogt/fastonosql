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

#include "gui/explorer/explorer_tree_view.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/utils_qt.h>        // for item

#include <common/qt/gui/regexp_input_dialog.h>

#include "proxy/cluster/icluster.h"       // for ICluster
#include "proxy/sentinel/isentinel.h"     // for Sentinel, etc
#include "proxy/server/iserver_remote.h"  // for IServer, IServerRemote
#include "proxy/settings_manager.h"       // for SettingsManager

#include "gui/dialogs/dbkey_dialog.h"           // for DbKeyDialog
#include "gui/dialogs/history_server_dialog.h"  // for ServerHistoryDialog
#include "gui/dialogs/info_server_dialog.h"     // for InfoServerDialog
#include "gui/dialogs/load_contentdb_dialog.h"  // for LoadContentDbDialog
#include "gui/dialogs/property_server_dialog.h"
#include "gui/dialogs/pub_sub_dialog.h"
#include "gui/dialogs/view_keys_dialog.h"  // for ViewKeysDialog

#include "gui/explorer/explorer_tree_item.h"
#include "gui/explorer/explorer_tree_model.h"  // for ExplorerServerItem, etc
#include "gui/explorer/explorer_tree_sort_filter_proxy_model.h"

#include "translations/global.h"  // for trClose, trBackup, trImport, etc

namespace {
const QString trCreateKeyForDbTemplate_1S = QObject::tr("Create key for %1 database");
const QString trEditKey_1S = QObject::tr("Edit key %1");
const QString trRemoveBranch = QObject::tr("Remove branch");
const QString trRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from branch %1?");
const QString trViewKeyTemplate_1S = QObject::tr("View keys in %1 database");
const QString trViewChannelsTemplate_1S = QObject::tr("View channels in %1 server");
const QString trConnectDisconnect = QObject::tr("Connect/Disconnect");
const QString trClearDb = QObject::tr("Clear database");
const QString trRealyRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from %1 database?");
const QString trLoadContentTemplate_1S = QObject::tr("Load keys in %1 database");
const QString trSetMaxConnectionOnServerTemplate_1S = QObject::tr("Set max connection on %1 server");
const QString trSetTTLOnKeyTemplate_1S = QObject::tr("Set ttl for %1 key");
const QString trNewTTLSeconds = QObject::tr("New TTL in seconds:");
const QString trSetIntervalOnKeyTemplate_1S = QObject::tr("Set watch interval for %1 key");
const QString trIntervalValue = QObject::tr("Interval msec:");
const QString trSetTTL = QObject::tr("Set TTL");
const QString trRemoveTTL = QObject::tr("Remove TTL");
const QString trRenameKey = QObject::tr("Rename key");
const QString trRenameKeyLabel = QObject::tr("New key name:");
const QString trCreateDatabase_1S = QObject::tr("Create database on %1 server");
const QString trRemoveDatabase_1S = QObject::tr("Remove database from %1 server");
}  // namespace

namespace fastonosql {
namespace gui {

ExplorerTreeView::ExplorerTreeView(QWidget* parent) : QTreeView(parent) {
  source_model_ = new ExplorerTreeModel(this);
  proxy_model_ = new ExplorerTreeSortFilterProxyModel(this);
  proxy_model_->setSourceModel(source_model_);
  proxy_model_->setDynamicSortFilter(true);
  proxy_model_->setSortRole(Qt::DisplayRole);
  setModel(proxy_model_);

  setSortingEnabled(true);
  sortByColumn(0, Qt::AscendingOrder);

  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &ExplorerTreeView::customContextMenuRequested, this, &ExplorerTreeView::showContextMenu));

  retranslateUi();
}

void ExplorerTreeView::addServer(proxy::IServerSPtr server) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  syncWithServer(server.get());
  source_model_->addServer(server);
}

void ExplorerTreeView::removeServer(proxy::IServerSPtr server) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  unsyncWithServer(server.get());
  source_model_->removeServer(server);
  emit serverClosed(server);
}

void ExplorerTreeView::addSentinel(proxy::ISentinelSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  proxy::ISentinel::sentinels_t nodes = sentinel->GetSentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    proxy::Sentinel sent = nodes[i];
    syncWithServer(sent.sentinel.get());
    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      syncWithServer(sent.sentinels_nodes[j].get());
    }
  }

  source_model_->addSentinel(sentinel);
}

void ExplorerTreeView::removeSentinel(proxy::ISentinelSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  proxy::ISentinel::sentinels_t nodes = sentinel->GetSentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    proxy::Sentinel sent = nodes[i];
    unsyncWithServer(sent.sentinel.get());
    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      unsyncWithServer(sent.sentinels_nodes[j].get());
    }
  }

  source_model_->removeSentinel(sentinel);
  emit sentinelClosed(sentinel);
}

void ExplorerTreeView::addCluster(proxy::IClusterSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  auto nodes = cluster->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    syncWithServer(nodes[i].get());
  }

  source_model_->addCluster(cluster);
}

void ExplorerTreeView::removeCluster(proxy::IClusterSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  auto nodes = cluster->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    unsyncWithServer(nodes[i].get());
  }

  source_model_->removeCluster(cluster);
  emit clusterClosed(cluster);
}

void ExplorerTreeView::changeTextFilter(const QString& text) {
  QRegExp regExp(text);
  proxy_model_->setFilterRegExp(regExp);
}

void ExplorerTreeView::showContextMenu(const QPoint& point) {
  QModelIndexList selected = selectedEqualTypeIndexes();
  if (selected.empty()) {
    return;
  }

  const QModelIndex index = selected[0];
  const bool is_multi = selected.size() > 1;
  UNUSED(is_multi);
  IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(index);
  if (!node) {
    DNOTREACHED();
    return;
  }

  QPoint menuPoint = mapToGlobal(point);
  menuPoint.setY(menuPoint.y() + header()->height());
  if (node->type() == IExplorerTreeItem::eCluster) {
    QMenu menu(this);
    QAction* closeClusterAction = new QAction(translations::trClose, this);
    VERIFY(connect(closeClusterAction, &QAction::triggered, this, &ExplorerTreeView::closeClusterConnection));
    menu.addAction(closeClusterAction);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eSentinel) {
    QMenu menu(this);
    QAction* closeSentinelAction = new QAction(translations::trClose, this);
    VERIFY(connect(closeSentinelAction, &QAction::triggered, this, &ExplorerTreeView::closeSentinelConnection));
    menu.addAction(closeSentinelAction);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eServer) {
    ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);

    QMenu menu(this);
    QAction* connectAction = new QAction(trConnectDisconnect, this);
    VERIFY(connect(connectAction, &QAction::triggered, this, &ExplorerTreeView::connectDisconnectToServer));

    QAction* openConsoleAction = new QAction(translations::trOpenConsole, this);
    VERIFY(connect(openConsoleAction, &QAction::triggered, this, &ExplorerTreeView::openConsole));

    menu.addAction(connectAction);
    menu.addAction(openConsoleAction);
    proxy::IServerSPtr server = server_node->server();
    bool is_connected = server->IsConnected();
    bool is_redis_compatible = core::IsRedisCompatible(server->GetType());

    common::qt::gui::TreeItem* par = node->parent();
    bool is_cluster_member = dynamic_cast<ExplorerClusterItem*>(par) != nullptr;  // +

    QAction* loadDatabaseAction = new QAction(translations::trLoadDataBases, this);
    VERIFY(connect(loadDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

    QAction* infoServerAction = new QAction(translations::trInfo, this);
    VERIFY(connect(infoServerAction, &QAction::triggered, this, &ExplorerTreeView::openInfoServerDialog));

    loadDatabaseAction->setEnabled(is_connected);
    menu.addAction(loadDatabaseAction);

    if (server->IsCanCreateDatabase()) {
      QAction* createDatabaseAction = new QAction(translations::trCreateDatabase, this);
      VERIFY(connect(createDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::createDb));
      createDatabaseAction->setEnabled(is_connected);
      menu.addAction(createDatabaseAction);
    }

    if (server->IsCanRemoveDatabase()) {
      QAction* removeDatabaseAction = new QAction(translations::trRemoveDatabase, this);
      VERIFY(connect(removeDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::removeDB));
      removeDatabaseAction->setEnabled(is_connected);
      menu.addAction(removeDatabaseAction);
    }

    infoServerAction->setEnabled(is_connected);
    menu.addAction(infoServerAction);

    if (is_redis_compatible) {
      QAction* propertyServerAction = new QAction(translations::trProperty, this);
      VERIFY(connect(propertyServerAction, &QAction::triggered, this, &ExplorerTreeView::openPropertyServerDialog));

      QAction* pubSubAction = new QAction(translations::trPubSubDialog, this);
      VERIFY(connect(pubSubAction, &QAction::triggered, this, &ExplorerTreeView::viewPubSub));

      propertyServerAction->setEnabled(is_connected);
      menu.addAction(propertyServerAction);

      pubSubAction->setEnabled(is_connected);
      menu.addAction(pubSubAction);

      bool is_local = true;
      bool is_can_remote = server->IsCanRemote();
      if (is_can_remote) {
        proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server.get());  // +
        CHECK(rserver);
        common::net::HostAndPort host = rserver->GetHost();
        is_local = host.IsLocalHost();  // failed if ssh connection
      }

      QAction* exportAction = new QAction(translations::trBackup, this);
      VERIFY(connect(exportAction, &QAction::triggered, this, &ExplorerTreeView::exportServer));

      QAction* importAction = new QAction(translations::trRestore, this);
      VERIFY(connect(importAction, &QAction::triggered, this, &ExplorerTreeView::importServer));

      exportAction->setEnabled(!is_connected && is_local);
      menu.addAction(exportAction);
      importAction->setEnabled(is_connected && is_local);
      menu.addAction(importAction);
    }

    QAction* historyServerAction = new QAction(translations::trHistory, this);
    VERIFY(connect(historyServerAction, &QAction::triggered, this, &ExplorerTreeView::openHistoryServerDialog));

    QAction* clearHistoryServerAction = new QAction(translations::trClearHistory, this);
    VERIFY(connect(clearHistoryServerAction, &QAction::triggered, this, &ExplorerTreeView::clearHistory));

    QAction* closeServerAction = new QAction(translations::trClose, this);
    VERIFY(connect(closeServerAction, &QAction::triggered, this, &ExplorerTreeView::closeServerConnection));

    menu.addAction(historyServerAction);
    menu.addAction(clearHistoryServerAction);
    closeServerAction->setEnabled(!is_cluster_member);
    menu.addAction(closeServerAction);

    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eDatabase) {
    ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);

    QMenu menu(this);
    QAction* loadContentAction = new QAction(translations::trLoadContOfDataBases, this);
    VERIFY(connect(loadContentAction, &QAction::triggered, this, &ExplorerTreeView::loadContentDb));

    QAction* createKeyAction = new QAction(translations::trCreateKey, this);
    VERIFY(connect(createKeyAction, &QAction::triggered, this, &ExplorerTreeView::createKey));

    QAction* viewKeysAction = new QAction(translations::trViewKeysDialog, this);
    VERIFY(connect(viewKeysAction, &QAction::triggered, this, &ExplorerTreeView::viewKeys));

    QAction* removeAllKeysAction = new QAction(translations::trRemoveAllKeys, this);
    VERIFY(connect(removeAllKeysAction, &QAction::triggered, this, &ExplorerTreeView::removeAllKeys));

    QAction* setDefaultDbAction = new QAction(translations::trSetDefault, this);
    VERIFY(connect(setDefaultDbAction, &QAction::triggered, this, &ExplorerTreeView::setDefaultDb));

    menu.addAction(loadContentAction);
    bool is_default = db->isDefault();
    proxy::IServerSPtr server = db->server();

    bool is_connected = server->IsConnected();
    loadContentAction->setEnabled(is_default && is_connected);

    menu.addAction(createKeyAction);
    createKeyAction->setEnabled(is_default && is_connected);

    menu.addAction(viewKeysAction);
    viewKeysAction->setEnabled(is_default && is_connected);

    menu.addAction(removeAllKeysAction);
    removeAllKeysAction->setEnabled(is_default && is_connected);

    menu.addAction(setDefaultDbAction);
    setDefaultDbAction->setEnabled(!is_default && is_connected);

    if (server->IsCanCreateDatabase()) {
      QAction* removeDatabaseAction = new QAction(translations::trDelete, this);
      VERIFY(connect(removeDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::removeDb));
      removeDatabaseAction->setEnabled(!is_default && is_connected);
      menu.addAction(removeDatabaseAction);
    }
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eNamespace) {
    ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);

    QMenu menu(this);
    QAction* removeBranchAction = new QAction(translations::trRemoveBranch, this);
    VERIFY(connect(removeBranchAction, &QAction::triggered, this, &ExplorerTreeView::removeBranch));

    proxy::IServerSPtr server = ns->server();
    ExplorerDatabaseItem* db = ns->db();
    bool is_default = db && db->isDefault();
    bool is_connected = server->IsConnected();

    menu.addAction(removeBranchAction);
    removeBranchAction->setEnabled(is_default && is_connected);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eKey) {
    ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);

    QMenu menu(this);
    QAction* getValueAction = new QAction(translations::trGetValue, this);
    VERIFY(connect(getValueAction, &QAction::triggered, this, &ExplorerTreeView::loadValue));

    QAction* editKeyAction = new QAction(translations::trEdit, this);
    VERIFY(connect(editKeyAction, &QAction::triggered, this, &ExplorerTreeView::editKey));

    QAction* renameKeyAction = new QAction(trRenameKey, this);
    VERIFY(connect(renameKeyAction, &QAction::triggered, this, &ExplorerTreeView::renKey));

    QAction* deleteKeyAction = new QAction(translations::trDelete, this);
    VERIFY(connect(deleteKeyAction, &QAction::triggered, this, &ExplorerTreeView::deleteKey));

    QAction* watchKeyAction = new QAction(translations::trWatch, this);
    VERIFY(connect(watchKeyAction, &QAction::triggered, this, &ExplorerTreeView::watchKey));

    proxy::IServerSPtr server = key->server();

    bool is_connected = server->IsConnected();
    menu.addAction(getValueAction);
    getValueAction->setEnabled(is_connected);
    bool is_ttl_supported = server->IsSupportTTLKeys();
    if (is_ttl_supported) {
      QAction* setTTLKeyAction = new QAction(trSetTTL, this);
      setTTLKeyAction->setEnabled(is_connected);
      VERIFY(connect(setTTLKeyAction, &QAction::triggered, this, &ExplorerTreeView::setTTL));
      menu.addAction(setTTLKeyAction);

      QAction* removeTTLKeyAction = new QAction(trRemoveTTL, this);
      removeTTLKeyAction->setEnabled(is_connected);
      VERIFY(connect(removeTTLKeyAction, &QAction::triggered, this, &ExplorerTreeView::removeTTL));
      menu.addAction(removeTTLKeyAction);
    }
    menu.addAction(renameKeyAction);
    renameKeyAction->setEnabled(is_connected);
    menu.addAction(editKeyAction);
    editKeyAction->setEnabled(is_connected);
    menu.addAction(deleteKeyAction);
    deleteKeyAction->setEnabled(is_connected);
    menu.addAction(watchKeyAction);
    watchKeyAction->setEnabled(is_connected);
    menu.exec(menuPoint);
  }
}

void ExplorerTreeView::connectDisconnectToServer() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      continue;
    }

    if (server->IsConnected()) {
      proxy::events_info::DisConnectInfoRequest req(this);
      server->Disconnect(req);
    } else {
      proxy::events_info::ConnectInfoRequest req(this);
      server->Connect(req);
    }
  }
}

void ExplorerTreeView::openConsole() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    emit consoleOpened(node->server(), QString());
  }
}

void ExplorerTreeView::loadDatabases() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    node->loadDatabases();
  }
}

void ExplorerTreeView::createDb() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }
    bool ok;
    QString name = QInputDialog::getText(this, trCreateDatabase_1S.arg(node->name()), translations::trName + ":",
                                         QLineEdit::Normal, QString(), &ok, Qt::WindowCloseButtonHint);
    if (ok && !name.isEmpty()) {
      node->createDatabase(name);
    }
  }
}

void ExplorerTreeView::removeDB() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }
    bool ok;
    QString name = QInputDialog::getText(this, trRemoveDatabase_1S.arg(node->name()), translations::trName + ":",
                                         QLineEdit::Normal, QString(), &ok, Qt::WindowCloseButtonHint);
    if (ok && !name.isEmpty()) {
      node->removeDatabase(name);
    }
  }
}

void ExplorerTreeView::openInfoServerDialog() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      continue;
    }

    InfoServerDialog infDialog(server, this);
    infDialog.exec();
  }
}

void ExplorerTreeView::openPropertyServerDialog() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      continue;
    }

    PropertyServerDialog infDialog(server, this);
    infDialog.exec();
  }
}

void ExplorerTreeView::openHistoryServerDialog() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      continue;
    }

    ServerHistoryDialog histDialog(server, this);
    histDialog.exec();
  }
}

void ExplorerTreeView::clearHistory() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      continue;
    }

    proxy::events_info::ClearServerHistoryRequest req(this);
    server->ClearHistory(req);
  }
}

void ExplorerTreeView::closeServerConnection() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* snode = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (snode) {
      proxy::IServerSPtr server = snode->server();
      if (server) {
        removeServer(server);
      }
      continue;
    }

    ExplorerClusterItem* cnode = common::qt::item<common::qt::gui::TreeItem*, ExplorerClusterItem*>(ind);
    if (cnode && cnode->type() == IExplorerTreeItem::eCluster) {
      proxy::IClusterSPtr server = cnode->cluster();
      if (server) {
        removeCluster(server);
      }
      continue;
    }
  }
}

void ExplorerTreeView::closeClusterConnection() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerClusterItem* cnode = common::qt::item<common::qt::gui::TreeItem*, ExplorerClusterItem*>(ind);
    if (!cnode) {
      continue;
    }

    proxy::IClusterSPtr server = cnode->cluster();
    if (server) {
      removeCluster(server);
    }
  }
}

void ExplorerTreeView::closeSentinelConnection() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerSentinelItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerSentinelItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::ISentinelSPtr sent = node->sentinel();
    if (sent) {
      removeSentinel(sent);
    }
  }
}

void ExplorerTreeView::viewPubSub() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    PubSubDialog diag(trViewChannelsTemplate_1S.arg(node->name()), server, this);
    VERIFY(connect(&diag, &PubSubDialog::consoleOpenedAndExecute, this, &ExplorerTreeView::consoleOpenedAndExecute));
    diag.exec();
  }
}

void ExplorerTreeView::importServer() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }
    proxy::IServerSPtr server = node->server();
    if (!server) {
      DNOTREACHED();
      break;
    }

    QString filepath =
        QFileDialog::getOpenFileName(this, translations::trBackup, QString(), translations::trfilterForRdb);
    if (!filepath.isEmpty()) {
      proxy::events_info::BackupInfoRequest req(this, common::ConvertToString(filepath));
      server->BackupToPath(req);
    }
  }
}

void ExplorerTreeView::exportServer() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (!server) {
      DNOTREACHED();
      break;
    }

    QString filepath =
        QFileDialog::getOpenFileName(this, translations::trImport, QString(), translations::trfilterForRdb);
    if (!filepath.isEmpty()) {
      proxy::events_info::RestoreInfoRequest req(this, common::ConvertToString(filepath));
      server->RestoreFromPath(req);
    }
  }
}

void ExplorerTreeView::loadContentDb() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    LoadContentDbDialog loadDb(trLoadContentTemplate_1S.arg(node->name()), node->server()->GetType(), this);
    int result = loadDb.exec();
    if (result == QDialog::Accepted) {
      node->loadContent(common::ConvertToString(loadDb.pattern()), loadDb.count());
    }
  }
}

void ExplorerTreeView::removeAllKeys() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    int answer = QMessageBox::question(this, trClearDb, trRealyRemoveAllKeysTemplate_1S.arg(node->name()),
                                       QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
      continue;
    }

    node->removeAllKeys();
  }
}

void ExplorerTreeView::removeBranch() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerNSItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    int answer = QMessageBox::question(this, trRemoveBranch, trRemoveAllKeysTemplate_1S.arg(node->name()),
                                       QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
      continue;
    }

    node->removeBranch();
  }
}

void ExplorerTreeView::setDefaultDb() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    node->setDefault();
  }
}

void ExplorerTreeView::removeDb() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    node->removeDb();
  }
}

void ExplorerTreeView::createKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    DbKeyDialog loadDb(trCreateKeyForDbTemplate_1S.arg(node->name()), server->GetType(), core::NDbKValue(), this);
    int result = loadDb.exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb.GetKey();
      node->createKey(key);
    }
  }
}

void ExplorerTreeView::editKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    DbKeyDialog loadDb(trEditKey_1S.arg(node->name()), server->GetType(), node->dbv(), this);
    int result = loadDb.exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb.GetKey();
      node->editKey(key.GetValue());
    }
  }
}

void ExplorerTreeView::viewKeys() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    ViewKeysDialog diag(trViewKeyTemplate_1S.arg(node->name()), node->db(), this);
    diag.exec();
  }
}

void ExplorerTreeView::loadValue() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      continue;
    }

    node->loadValueFromDb();
  }
}

void ExplorerTreeView::renKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    QString name = node->name();
    common::qt::gui::RegExpInputDialog reg_dialog(this);
    reg_dialog.setWindowTitle(trRenameKey);
    reg_dialog.setLabelText(trRenameKeyLabel);
    reg_dialog.setText(name);
    QRegExp regExp(".*");
    reg_dialog.setRegExp(regExp);
    int result = reg_dialog.exec();
    if (result != QDialog::Accepted) {
      continue;
    }

    QString new_key_name = reg_dialog.text();
    if (new_key_name.isEmpty()) {
      continue;
    }

    if (new_key_name == name) {
      continue;
    }

    node->renameKey(new_key_name);
  }
}

void ExplorerTreeView::deleteKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    node->removeFromDb();
  }
}

void ExplorerTreeView::watchKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    bool ok;
    QString name = node->name();
    int interval = QInputDialog::getInt(this, trSetIntervalOnKeyTemplate_1S.arg(name), trIntervalValue, 1000, 0,
                                        INT32_MAX, 1000, &ok, Qt::WindowCloseButtonHint);
    if (ok) {
      node->watchKey(interval);
    }
  }
}

void ExplorerTreeView::setTTL() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    bool ok;
    QString name = node->name();
    core::NKey key = node->key();
    int ttl = QInputDialog::getInt(this, trSetTTLOnKeyTemplate_1S.arg(name), trNewTTLSeconds, key.GetTTL(), NO_TTL,
                                   INT32_MAX, 100, &ok, Qt::WindowCloseButtonHint);
    if (ok) {
      node->setTTL(ttl);
    }
  }
}

void ExplorerTreeView::removeTTL() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    node->setTTL(NO_TTL);
  }
}

void ExplorerTreeView::startLoadDatabases(const proxy::events_info::LoadDatabasesInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabases(const proxy::events_info::LoadDatabasesInfoResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  proxy::events_info::LoadDatabasesInfoResponce::database_info_cont_type dbs = res.databases;
  for (size_t i = 0; i < dbs.size(); ++i) {
    core::IDataBaseInfoSPtr db = dbs[i];
    source_model_->addDatabase(serv, db);
  }
}

void ExplorerTreeView::startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  proxy::events_info::LoadDatabaseContentResponce::keys_container_t keys = res.keys;
  const std::string ns = serv->GetNsSeparator();
  core::NsDisplayStrategy ns_strategy = serv->GetNsDisplayStrategy();
  for (size_t i = 0; i < keys.size(); ++i) {
    core::NDbKValue key = keys[i];
    source_model_->addKey(serv, res.inf, key, ns, ns_strategy);
  }

  source_model_->updateDb(serv, res.inf);
}

void ExplorerTreeView::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}
void ExplorerTreeView::createDatabase(core::IDataBaseInfoSPtr db) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->addDatabase(serv, db);
}

void ExplorerTreeView::removeDatabase(core::IDataBaseInfoSPtr db) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->removeDatabase(serv, db);
}

void ExplorerTreeView::flushDB(core::IDataBaseInfoSPtr db) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->removeAllKeys(serv, db);
}

void ExplorerTreeView::currentDataBaseChange(core::IDataBaseInfoSPtr db) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->addDatabase(serv, db);
  source_model_->setDefaultDb(serv, db);
}

void ExplorerTreeView::removeKey(core::IDataBaseInfoSPtr db, core::NKey key) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->removeKey(serv, db, key);
}

void ExplorerTreeView::addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  const std::string ns = serv->GetNsSeparator();
  const core::NsDisplayStrategy ns_strategy = serv->GetNsDisplayStrategy();
  source_model_->addKey(serv, db, key, ns, ns_strategy);
}

void ExplorerTreeView::renameKey(core::IDataBaseInfoSPtr db, core::NKey key, core::key_t new_name) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  core::NKey new_key = key;
  new_key.SetKey(new_name);
  source_model_->updateKey(serv, db, key, new_key);
}

void ExplorerTreeView::loadKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  source_model_->updateValue(serv, db, key);
}

void ExplorerTreeView::changeTTLKey(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  core::NKey new_key = key;
  new_key.SetTTL(ttl);
  source_model_->updateKey(serv, db, key, new_key);
}

void ExplorerTreeView::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QTreeView::changeEvent(e);
}

void ExplorerTreeView::mouseDoubleClickEvent(QMouseEvent* e) {
  if (proxy::SettingsManager::GetInstance()->GetFastViewKeys()) {
    loadValue();
  }

  QTreeView::mouseDoubleClickEvent(e);
}

void ExplorerTreeView::syncWithServer(proxy::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(connect(server, &proxy::IServer::LoadDatabasesStarted, this, &ExplorerTreeView::startLoadDatabases));
  VERIFY(connect(server, &proxy::IServer::LoadDatabasesFinished, this, &ExplorerTreeView::finishLoadDatabases));
  VERIFY(
      connect(server, &proxy::IServer::LoadDataBaseContentStarted, this, &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(connect(server, &proxy::IServer::LoadDatabaseContentFinished, this,
                 &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(connect(server, &proxy::IServer::ExecuteStarted, this, &ExplorerTreeView::startExecuteCommand));
  VERIFY(connect(server, &proxy::IServer::ExecuteFinished, this, &ExplorerTreeView::finishExecuteCommand));

  VERIFY(connect(server, &proxy::IServer::DatabaseRemoved, this, &ExplorerTreeView::removeDatabase));
  VERIFY(connect(server, &proxy::IServer::DatabaseCreated, this, &ExplorerTreeView::createDatabase));
  VERIFY(connect(server, &proxy::IServer::DatabaseFlushed, this, &ExplorerTreeView::flushDB));
  VERIFY(connect(server, &proxy::IServer::DatabaseChanged, this, &ExplorerTreeView::currentDataBaseChange));

  VERIFY(connect(server, &proxy::IServer::KeyRemoved, this, &ExplorerTreeView::removeKey, Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyAdded, this, &ExplorerTreeView::addKey, Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyRenamed, this, &ExplorerTreeView::renameKey, Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyLoaded, this, &ExplorerTreeView::loadKey, Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyTTLChanged, this, &ExplorerTreeView::changeTTLKey, Qt::DirectConnection));
}

void ExplorerTreeView::unsyncWithServer(proxy::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(disconnect(server, &proxy::IServer::LoadDatabasesStarted, this, &ExplorerTreeView::startLoadDatabases));
  VERIFY(disconnect(server, &proxy::IServer::LoadDatabasesFinished, this, &ExplorerTreeView::finishLoadDatabases));
  VERIFY(disconnect(server, &proxy::IServer::LoadDataBaseContentStarted, this,
                    &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(disconnect(server, &proxy::IServer::LoadDatabaseContentFinished, this,
                    &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(disconnect(server, &proxy::IServer::ExecuteStarted, this, &ExplorerTreeView::startExecuteCommand));
  VERIFY(disconnect(server, &proxy::IServer::ExecuteFinished, this, &ExplorerTreeView::finishExecuteCommand));

  VERIFY(disconnect(server, &proxy::IServer::DatabaseRemoved, this, &ExplorerTreeView::removeDatabase));
  VERIFY(disconnect(server, &proxy::IServer::DatabaseCreated, this, &ExplorerTreeView::createDatabase));
  VERIFY(disconnect(server, &proxy::IServer::DatabaseFlushed, this, &ExplorerTreeView::flushDB));
  VERIFY(disconnect(server, &proxy::IServer::DatabaseChanged, this, &ExplorerTreeView::currentDataBaseChange));

  VERIFY(disconnect(server, &proxy::IServer::KeyRemoved, this, &ExplorerTreeView::removeKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyAdded, this, &ExplorerTreeView::addKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyRenamed, this, &ExplorerTreeView::renameKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyLoaded, this, &ExplorerTreeView::loadKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyTTLChanged, this, &ExplorerTreeView::changeTTLKey));
}

void ExplorerTreeView::retranslateUi() {}

QModelIndexList ExplorerTreeView::selectedEqualTypeIndexes() const {
  QModelIndexList indexses = selectionModel()->selectedRows();
  if (indexses.empty()) {
    return QModelIndexList();
  }

  const QModelIndex first = indexses[0];
  IExplorerTreeItem* first_node =
      common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(proxy_model_->mapToSource(first));
  if (!first_node) {
    DNOTREACHED();
    return QModelIndexList();
  }

  QModelIndexList selected;
  for (QModelIndex ind : indexses) {
    QModelIndex pr = proxy_model_->mapToSource(ind);
    IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(pr);
    if (!node) {
      DNOTREACHED();
      return QModelIndexList();
    }

    IExplorerTreeItem::eType cur_typ = node->type();
    if (first_node->type() != cur_typ) {
      return QModelIndexList();
    }
    selected.push_back(pr);
  }
  return selected;
}

}  // namespace gui
}  // namespace fastonosql
