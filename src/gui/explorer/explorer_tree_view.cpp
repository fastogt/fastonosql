/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for INT32_MAX
#include <memory>    // for __shared_ptr
#include <string>    // for string
#include <vector>    // for vector

#include <QAction>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/error.h>              // for Error
#include <common/macros.h>             // for VERIFY, CHECK, DNOTREACHED, etc
#include <common/net/types.h>          // for HostAndPort
#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/utils_qt.h>        // for item
#include <common/value.h>              // for ErrorValue

#include <common/qt/gui/base/tree_item.h>  // for TreeItem
#include <common/qt/gui/regexp_input_dialog.h>

#include "core/connection_types.h"        // for connectionTypes::REDIS
#include "core/db_key.h"                  // for NDbKValue
#include "proxy/cluster/icluster.h"       // for ICluster
#include "proxy/events/events_info.h"     // for CommandResponce, etc
#include "proxy/sentinel/isentinel.h"     // for Sentinel, etc
#include "proxy/server/iserver_remote.h"  // for IServer, IServerRemote
#include "proxy/settings_manager.h"       // for SettingsManager

#include "gui/dialogs/change_password_server_dialog.h"
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
const QString trViewKeyTemplate_1S = QObject::tr("View key in %1 database");
const QString trViewChannelsTemplate_1S = QObject::tr("View channels in %1 server");
const QString trConnectDisconnect = QObject::tr("Connect/Disconnect");
const QString trClearDb = QObject::tr("Clear database");
const QString trRealyRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from %1 database?");
const QString trLoadContentTemplate_1S = QObject::tr("Load %1 content");
const QString trReallyShutdownTemplate_1S = QObject::tr("Really shutdown \"%1\" server?");
const QString trSetMaxConnectionOnServerTemplate_1S = QObject::tr("Set max connection on %1 server");
const QString trMaximumConnectionTemplate = QObject::tr("Maximum connection:");
const QString trSetTTLOnKeyTemplate_1S = QObject::tr("Set ttl for %1 key");
const QString trTTLValue = QObject::tr("New TTL:");
const QString trSetIntervalOnKeyTemplate_1S = QObject::tr("Set watch interval for %1 key");
const QString trIntervalValue = QObject::tr("Interval msec:");
const QString trSetTTL = QObject::tr("Set TTL");
const QString trRenameKey = QObject::tr("Rename key");
const QString trRenameKeyLabel = QObject::tr("New key name:");
const QString trChangePasswordTemplate_1S = QObject::tr("Change password for %1 server");
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

  proxy::ISentinel::sentinels_t nodes = sentinel->Sentinels();
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

  proxy::ISentinel::sentinels_t nodes = sentinel->Sentinels();
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

  auto nodes = cluster->Nodes();
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

  auto nodes = cluster->Nodes();
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
    bool is_redis = server->Type() == core::REDIS;

    bool is_cluster_member = dynamic_cast<ExplorerClusterItem*>(node->parent()) != nullptr;  // +

    QAction* loadDatabaseAction = new QAction(translations::trLoadDataBases, this);
    VERIFY(connect(loadDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

    QAction* infoServerAction = new QAction(translations::trInfo, this);
    VERIFY(connect(infoServerAction, &QAction::triggered, this, &ExplorerTreeView::openInfoServerDialog));

    loadDatabaseAction->setEnabled(is_connected);
    menu.addAction(loadDatabaseAction);
    infoServerAction->setEnabled(is_connected);
    menu.addAction(infoServerAction);

    if (is_redis) {
      QAction* propertyServerAction = new QAction(translations::trProperty, this);
      VERIFY(connect(propertyServerAction, &QAction::triggered, this, &ExplorerTreeView::openPropertyServerDialog));

      QAction* setServerPassword = new QAction(translations::trSetPassword, this);
      VERIFY(connect(setServerPassword, &QAction::triggered, this, &ExplorerTreeView::openSetPasswordServerDialog));

      QAction* setMaxClientConnection = new QAction(translations::trSetMaxNumberOfClients, this);
      VERIFY(connect(setMaxClientConnection, &QAction::triggered, this, &ExplorerTreeView::openMaxClientSetDialog));

      QAction* pubSubAction = new QAction(translations::trPubSubDialog, this);
      VERIFY(connect(pubSubAction, &QAction::triggered, this, &ExplorerTreeView::viewPubSub));

      propertyServerAction->setEnabled(is_connected);
      menu.addAction(propertyServerAction);

      setServerPassword->setEnabled(is_connected);
      menu.addAction(setServerPassword);

      setMaxClientConnection->setEnabled(is_connected);
      menu.addAction(setMaxClientConnection);

      pubSubAction->setEnabled(is_connected);
      menu.addAction(pubSubAction);

      bool is_can_remote = server->IsCanRemote();
      bool is_local = true;
      if (is_can_remote) {
        proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server.get());  // +
        CHECK(rserver);
        common::net::HostAndPort host = rserver->Host();
        is_local = host.IsLocalHost();
      }

      QAction* importAction = new QAction(translations::trImport, this);
      VERIFY(connect(importAction, &QAction::triggered, this, &ExplorerTreeView::importServer));

      QAction* backupAction = new QAction(translations::trBackup, this);
      VERIFY(connect(backupAction, &QAction::triggered, this, &ExplorerTreeView::backupServer));

      QAction* shutdownAction = new QAction(translations::trShutdown, this);
      VERIFY(connect(shutdownAction, &QAction::triggered, this, &ExplorerTreeView::shutdownServer));

      importAction->setEnabled(!is_connected && is_local);
      menu.addAction(importAction);
      backupAction->setEnabled(is_connected && is_local);
      menu.addAction(backupAction);
      shutdownAction->setEnabled(is_connected);
      menu.addAction(shutdownAction);
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
    bool is_TTL_supported = server->IsSupportTTLKeys();
    if (is_TTL_supported) {
      QAction* setTTLKeyAction = new QAction(trSetTTL, this);
      setTTLKeyAction->setEnabled(is_connected);
      VERIFY(connect(setTTLKeyAction, &QAction::triggered, this, &ExplorerTreeView::setTTL));
      menu.addAction(setTTLKeyAction);
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

void ExplorerTreeView::openSetPasswordServerDialog() {
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

    ChangePasswordServerDialog pass(trChangePasswordTemplate_1S.arg(node->name()), server, this);
    pass.exec();
  }
}

void ExplorerTreeView::openMaxClientSetDialog() {
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

    bool ok;
    QString name;
    common::ConvertFromString(server->Name(), &name);
    int maxcl = QInputDialog::getInt(this, trSetMaxConnectionOnServerTemplate_1S.arg(name), trMaximumConnectionTemplate,
                                     10000, 1, INT32_MAX, 100, &ok);
    if (ok) {
      proxy::events_info::ChangeMaxConnectionRequest req(this, maxcl);
      server->SetMaxConnection(req);
    }
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

void ExplorerTreeView::backupServer() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    QString filepath =
        QFileDialog::getOpenFileName(this, translations::trBackup, QString(), translations::trfilterForRdb);
    if (!filepath.isEmpty() && server) {
      proxy::events_info::BackupInfoRequest req(this, common::ConvertToString(filepath));
      server->BackupToPath(req);
    }
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
    QString filepath =
        QFileDialog::getOpenFileName(this, translations::trImport, QString(), translations::trfilterForRdb);
    if (filepath.isEmpty() && server) {
      proxy::events_info::ExportInfoRequest req(this, common::ConvertToString(filepath));
      server->ExportFromPath(req);
    }
  }
}

void ExplorerTreeView::shutdownServer() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    if (server && server->IsConnected()) {
      // Ask user
      QString name;
      common::ConvertFromString(server->Name(), &name);
      int answer = QMessageBox::question(this, translations::trShutdown, trReallyShutdownTemplate_1S.arg(name),
                                         QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (answer != QMessageBox::Yes) {
        continue;
      }

      proxy::events_info::ShutDownInfoRequest req(this);
      server->ShutDown(req);
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

    LoadContentDbDialog loadDb(trLoadContentTemplate_1S.arg(node->name()), node->server()->Type(), this);
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

void ExplorerTreeView::createKey() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerDatabaseItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    DbKeyDialog loadDb(trCreateKeyForDbTemplate_1S.arg(node->name()), server->Type(), core::NDbKValue(), this);
    int result = loadDb.exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb.key();
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
    DbKeyDialog loadDb(trEditKey_1S.arg(node->name()), server->Type(), node->dbv(), this);
    int result = loadDb.exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb.key();
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
    QRegExp regExp("\\S+");
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
                                        INT32_MAX, 1000, &ok);
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
    int ttl = QInputDialog::getInt(this, trSetTTLOnKeyTemplate_1S.arg(name), trTTLValue, key.GetTTL(), -1, INT32_MAX,
                                   100, &ok);
    if (ok) {
      node->setTTL(ttl);
    }
  }
}

void ExplorerTreeView::startLoadDatabases(const proxy::events_info::LoadDatabasesInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabases(const proxy::events_info::LoadDatabasesInfoResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->IsError()) {
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
  common::Error er = res.errorInfo();
  if (er && er->IsError()) {
    return;
  }

  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  proxy::events_info::LoadDatabaseContentResponce::keys_container_t keys = res.keys;
  std::string ns = serv->NsSeparator();
  for (size_t i = 0; i < keys.size(); ++i) {
    core::NDbKValue key = keys[i];
    source_model_->addKey(serv, res.inf, key, ns);
  }

  source_model_->updateDb(serv, res.inf);
}

void ExplorerTreeView::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
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

  std::string ns = serv->NsSeparator();
  source_model_->addKey(serv, db, key, ns);
}

void ExplorerTreeView::renameKey(core::IDataBaseInfoSPtr db, core::NKey key, std::string new_name) {
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
  if (proxy::SettingsManager::Instance().FastViewKeys()) {
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

  VERIFY(connect(server, &proxy::IServer::FlushedDB, this, &ExplorerTreeView::flushDB));
  VERIFY(connect(server, &proxy::IServer::CurrentDataBaseChanged, this, &ExplorerTreeView::currentDataBaseChange));
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

  VERIFY(disconnect(server, &proxy::IServer::FlushedDB, this, &ExplorerTreeView::flushDB));
  VERIFY(disconnect(server, &proxy::IServer::CurrentDataBaseChanged, this, &ExplorerTreeView::currentDataBaseChange));
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
