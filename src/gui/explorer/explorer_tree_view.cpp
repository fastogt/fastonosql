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

#include "gui/explorer/explorer_tree_view.h"

#include <memory>    // for __shared_ptr
#include <stddef.h>  // for size_t
#include <stdint.h>  // for INT32_MAX
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
#include "proxy/events/events_info.h"     // for CommandResponce, etc
#include "proxy/cluster/icluster.h"       // for ICluster
#include "proxy/sentinel/isentinel.h"     // for Sentinel, etc
#include "proxy/server/iserver_remote.h"  // for IServer, IServerRemote
#include "proxy/settings_manager.h"       // for SettingsManager

#include "gui/dialogs/change_password_server_dialog.h"
#include "gui/dialogs/dbkey_dialog.h"           // for DbKeyDialog
#include "gui/dialogs/history_server_dialog.h"  // for ServerHistoryDialog
#include "gui/dialogs/info_server_dialog.h"     // for InfoServerDialog
#include "gui/dialogs/load_contentdb_dialog.h"  // for LoadContentDbDialog
#include "gui/dialogs/property_server_dialog.h"
#include "gui/dialogs/view_keys_dialog.h"  // for ViewKeysDialog
#include "gui/dialogs/pub_sub_dialog.h"
#include "gui/explorer/explorer_tree_model.h"  // for ExplorerServerItem, etc
#include "gui/explorer/explorer_tree_sort_filter_proxy_model.h"
#include "gui/explorer/explorer_tree_item.h"

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
const QString trRealyRemoveAllKeysTemplate_1S =
    QObject::tr("Really remove all keys from %1 database?");
const QString trLoadContentTemplate_1S = QObject::tr("Load %1 content");
const QString trReallyShutdownTemplate_1S = QObject::tr("Really shutdown \"%1\" server?");
const QString trSetMaxConnectionOnServerTemplate_1S =
    QObject::tr("Set max connection on %1 server");
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
  setSelectionMode(QAbstractItemView::SingleSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &ExplorerTreeView::customContextMenuRequested, this,
                 &ExplorerTreeView::showContextMenu));

  connectAction_ = new QAction(this);
  VERIFY(connect(connectAction_, &QAction::triggered, this,
                 &ExplorerTreeView::connectDisconnectToServer));
  openConsoleAction_ = new QAction(this);
  VERIFY(connect(openConsoleAction_, &QAction::triggered, this, &ExplorerTreeView::openConsole));
  loadDatabaseAction_ = new QAction(this);
  VERIFY(connect(loadDatabaseAction_, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

  infoServerAction_ = new QAction(this);
  VERIFY(connect(infoServerAction_, &QAction::triggered, this,
                 &ExplorerTreeView::openInfoServerDialog));

  propertyServerAction_ = new QAction(this);
  VERIFY(connect(propertyServerAction_, &QAction::triggered, this,
                 &ExplorerTreeView::openPropertyServerDialog));

  setServerPassword_ = new QAction(this);
  VERIFY(connect(setServerPassword_, &QAction::triggered, this,
                 &ExplorerTreeView::openSetPasswordServerDialog));

  setMaxClientConnection_ = new QAction(this);
  VERIFY(connect(setMaxClientConnection_, &QAction::triggered, this,
                 &ExplorerTreeView::openMaxClientSetDialog));

  historyServerAction_ = new QAction(this);
  VERIFY(connect(historyServerAction_, &QAction::triggered, this,
                 &ExplorerTreeView::openHistoryServerDialog));

  clearHistoryServerAction_ = new QAction(this);
  VERIFY(connect(clearHistoryServerAction_, &QAction::triggered, this,
                 &ExplorerTreeView::clearHistory));

  closeServerAction_ = new QAction(this);
  VERIFY(connect(closeServerAction_, &QAction::triggered, this,
                 &ExplorerTreeView::closeServerConnection));

  closeClusterAction_ = new QAction(this);
  VERIFY(connect(closeClusterAction_, &QAction::triggered, this,
                 &ExplorerTreeView::closeClusterConnection));

  closeSentinelAction_ = new QAction(this);
  VERIFY(connect(closeSentinelAction_, &QAction::triggered, this,
                 &ExplorerTreeView::closeSentinelConnection));

  pubSubAction_ = new QAction(this);
  VERIFY(connect(pubSubAction_, &QAction::triggered, this, &ExplorerTreeView::viewPubSub));

  importAction_ = new QAction(this);
  VERIFY(connect(importAction_, &QAction::triggered, this, &ExplorerTreeView::importServer));

  backupAction_ = new QAction(this);
  VERIFY(connect(backupAction_, &QAction::triggered, this, &ExplorerTreeView::backupServer));

  shutdownAction_ = new QAction(this);
  VERIFY(connect(shutdownAction_, &QAction::triggered, this, &ExplorerTreeView::shutdownServer));

  loadContentAction_ = new QAction(this);
  VERIFY(connect(loadContentAction_, &QAction::triggered, this, &ExplorerTreeView::loadContentDb));

  removeAllKeysAction_ = new QAction(this);
  VERIFY(
      connect(removeAllKeysAction_, &QAction::triggered, this, &ExplorerTreeView::removeAllKeys));

  removeBranchAction_ = new QAction(this);
  VERIFY(connect(removeBranchAction_, &QAction::triggered, this, &ExplorerTreeView::removeBranch));

  setDefaultDbAction_ = new QAction(this);
  VERIFY(connect(setDefaultDbAction_, &QAction::triggered, this, &ExplorerTreeView::setDefaultDb));

  createKeyAction_ = new QAction(this);
  VERIFY(connect(createKeyAction_, &QAction::triggered, this, &ExplorerTreeView::createKey));

  editKeyAction_ = new QAction(this);
  VERIFY(connect(editKeyAction_, &QAction::triggered, this, &ExplorerTreeView::editKey));

  viewKeysAction_ = new QAction(this);
  VERIFY(connect(viewKeysAction_, &QAction::triggered, this, &ExplorerTreeView::viewKeys));

  getValueAction_ = new QAction(this);
  VERIFY(connect(getValueAction_, &QAction::triggered, this, &ExplorerTreeView::loadValue));

  renameKeyAction_ = new QAction(this);
  VERIFY(connect(renameKeyAction_, &QAction::triggered, this, &ExplorerTreeView::renKey));

  deleteKeyAction_ = new QAction(this);
  VERIFY(connect(deleteKeyAction_, &QAction::triggered, this, &ExplorerTreeView::deleteKey));

  watchKeyAction_ = new QAction(this);
  VERIFY(connect(watchKeyAction_, &QAction::triggered, this, &ExplorerTreeView::watchKey));

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
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(sel);
  if (!node) {
    DNOTREACHED();
    return;
  }

  QPoint menuPoint = mapToGlobal(point);
  menuPoint.setY(menuPoint.y() + header()->height());
  if (node->type() == IExplorerTreeItem::eCluster) {
    QMenu menu(this);
    closeClusterAction_->setEnabled(true);
    menu.addAction(closeClusterAction_);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eSentinel) {
    QMenu menu(this);
    closeSentinelAction_->setEnabled(true);
    menu.addAction(closeSentinelAction_);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eServer) {
    ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);

    QMenu menu(this);
    menu.addAction(connectAction_);
    menu.addAction(openConsoleAction_);
    proxy::IServerSPtr server = server_node->server();
    bool is_connected = server->IsConnected();
    bool is_redis = server->Type() == core::REDIS;

    bool isClusterMember = dynamic_cast<ExplorerClusterItem*>(node->parent()) != nullptr;  // +

    loadDatabaseAction_->setEnabled(is_connected);
    menu.addAction(loadDatabaseAction_);
    infoServerAction_->setEnabled(is_connected);
    menu.addAction(infoServerAction_);
    propertyServerAction_->setEnabled(is_connected && is_redis);
    menu.addAction(propertyServerAction_);

    setServerPassword_->setEnabled(is_connected && is_redis);
    menu.addAction(setServerPassword_);

    setMaxClientConnection_->setEnabled(is_connected && is_redis);
    menu.addAction(setMaxClientConnection_);

    menu.addAction(historyServerAction_);
    menu.addAction(clearHistoryServerAction_);
    closeServerAction_->setEnabled(!isClusterMember);
    menu.addAction(closeServerAction_);

    pubSubAction_->setEnabled(is_connected && is_redis);
    menu.addAction(pubSubAction_);

    bool is_can_remote = server->IsCanRemote();
    bool is_local = true;
    if (is_can_remote) {
      proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server.get());  // +
      CHECK(rserver);
      common::net::HostAndPort host = rserver->Host();
      is_local = host.isLocalHost();
    }

    importAction_->setEnabled(!is_connected && is_local && is_redis);
    menu.addAction(importAction_);
    backupAction_->setEnabled(is_connected && is_local && is_redis);
    menu.addAction(backupAction_);
    shutdownAction_->setEnabled(is_connected && is_redis);
    menu.addAction(shutdownAction_);

    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eDatabase) {
    ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);

    QMenu menu(this);
    menu.addAction(loadContentAction_);
    bool isDefault = db->isDefault();
    proxy::IServerSPtr server = db->server();

    bool is_connected = server->IsConnected();
    loadContentAction_->setEnabled(isDefault && is_connected);

    menu.addAction(createKeyAction_);
    createKeyAction_->setEnabled(isDefault && is_connected);

    menu.addAction(viewKeysAction_);
    viewKeysAction_->setEnabled(isDefault && is_connected);

    menu.addAction(removeAllKeysAction_);
    removeAllKeysAction_->setEnabled(isDefault && is_connected);

    menu.addAction(setDefaultDbAction_);
    setDefaultDbAction_->setEnabled(!isDefault && is_connected);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eNamespace) {
    ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);

    QMenu menu(this);
    proxy::IServerSPtr server = ns->server();
    ExplorerDatabaseItem* db = ns->db();
    bool isDefault = db && db->isDefault();
    bool is_connected = server->IsConnected();

    menu.addAction(removeBranchAction_);
    removeBranchAction_->setEnabled(isDefault && is_connected);
    menu.exec(menuPoint);
  } else if (node->type() == IExplorerTreeItem::eKey) {
    ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);

    QMenu menu(this);
    proxy::IServerSPtr server = key->server();

    bool is_connected = server->IsConnected();
    menu.addAction(getValueAction_);
    getValueAction_->setEnabled(is_connected);
    bool isTTLSupported = server->IsSupportTTLKeys();
    if (isTTLSupported) {
      QAction* setTTLKeyAction = new QAction(trSetTTL, this);
      setTTLKeyAction->setEnabled(is_connected);
      VERIFY(connect(setTTLKeyAction, &QAction::triggered, this, &ExplorerTreeView::setTTL));
      menu.addAction(setTTLKeyAction);
    }
    menu.addAction(renameKeyAction_);
    renameKeyAction_->setEnabled(is_connected);
    menu.addAction(editKeyAction_);
    editKeyAction_->setEnabled(is_connected);
    menu.addAction(deleteKeyAction_);
    deleteKeyAction_->setEnabled(is_connected);
    menu.addAction(watchKeyAction_);
    watchKeyAction_->setEnabled(is_connected);
    menu.exec(menuPoint);
  }
}

void ExplorerTreeView::connectDisconnectToServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  if (server->IsConnected()) {
    proxy::events_info::DisConnectInfoRequest req(this);
    server->Disconnect(req);
  } else {
    proxy::events_info::ConnectInfoRequest req(this);
    server->Connect(req);
  }
}

void ExplorerTreeView::openConsole() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (node) {
    emit consoleOpened(node->server(), QString());
  }
}

void ExplorerTreeView::loadDatabases() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (node) {
    node->loadDatabases();
  }
}

void ExplorerTreeView::openInfoServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  InfoServerDialog infDialog(server, this);
  infDialog.exec();
}

void ExplorerTreeView::openPropertyServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  PropertyServerDialog infDialog(server, this);
  infDialog.exec();
}

void ExplorerTreeView::openSetPasswordServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  ChangePasswordServerDialog pass(trChangePasswordTemplate_1S.arg(node->name()), server, this);
  pass.exec();
}

void ExplorerTreeView::openMaxClientSetDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  bool ok;
  QString name = common::ConvertFromString<QString>(server->Name());
  int maxcl = QInputDialog::getInt(this, trSetMaxConnectionOnServerTemplate_1S.arg(name),
                                   trMaximumConnectionTemplate, 10000, 1, INT32_MAX, 100, &ok);
  if (ok) {
    proxy::events_info::ChangeMaxConnectionRequest req(this, maxcl);
    server->SetMaxConnection(req);
  }
}

void ExplorerTreeView::openHistoryServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  ServerHistoryDialog histDialog(server, this);
  histDialog.exec();
}

void ExplorerTreeView::clearHistory() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  proxy::events_info::ClearServerHistoryRequest req(this);
  server->ClearHistory(req);
}

void ExplorerTreeView::closeServerConnection() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* snode =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (snode) {
    proxy::IServerSPtr server = snode->server();
    if (server) {
      removeServer(server);
    }
    return;
  }

  ExplorerClusterItem* cnode =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerClusterItem*>(sel);
  if (cnode && cnode->type() == IExplorerTreeItem::eCluster) {
    proxy::IClusterSPtr server = cnode->cluster();
    if (server) {
      removeCluster(server);
    }
    return;
  }
}

void ExplorerTreeView::closeClusterConnection() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerClusterItem* cnode =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerClusterItem*>(sel);
  if (!cnode) {
    return;
  }

  proxy::IClusterSPtr server = cnode->cluster();
  if (server) {
    removeCluster(server);
  }
}

void ExplorerTreeView::closeSentinelConnection() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerSentinelItem* snode =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerSentinelItem*>(sel);
  if (!snode) {
    return;
  }

  proxy::ISentinelSPtr sent = snode->sentinel();
  if (sent) {
    removeSentinel(sent);
  }
}

void ExplorerTreeView::viewPubSub() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  PubSubDialog diag(trViewChannelsTemplate_1S.arg(node->name()), server, this);
  VERIFY(connect(&diag, &PubSubDialog::consoleOpenedAndExecute, this,
                 &ExplorerTreeView::consoleOpenedAndExecute));
  diag.exec();
}

void ExplorerTreeView::backupServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  QString filepath = QFileDialog::getOpenFileName(this, translations::trBackup, QString(),
                                                  translations::trfilterForRdb);
  if (!filepath.isEmpty() && server) {
    proxy::events_info::BackupInfoRequest req(this, common::ConvertToString(filepath));
    server->BackupToPath(req);
  }
}

void ExplorerTreeView::importServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  QString filepath = QFileDialog::getOpenFileName(this, translations::trImport, QString(),
                                                  translations::trfilterForRdb);
  if (filepath.isEmpty() && server) {
    proxy::events_info::ExportInfoRequest req(this, common::ConvertToString(filepath));
    server->ExportFromPath(req);
  }
}

void ExplorerTreeView::shutdownServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  if (server && server->IsConnected()) {
    // Ask user
    QString name = common::ConvertFromString<QString>(server->Name());
    int answer =
        QMessageBox::question(this, translations::trShutdown, trReallyShutdownTemplate_1S.arg(name),
                              QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
      return;
    }

    proxy::events_info::ShutDownInfoRequest req(this);
    server->ShutDown(req);
  }
}

void ExplorerTreeView::loadContentDb() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (!node) {
    return;
  }

  LoadContentDbDialog loadDb(trLoadContentTemplate_1S.arg(node->name()), node->server()->Type(),
                             this);
  int result = loadDb.exec();
  if (result == QDialog::Accepted) {
    node->loadContent(common::ConvertToString(loadDb.pattern()), loadDb.count());
  }
}

void ExplorerTreeView::removeAllKeys() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (node) {
    int answer =
        QMessageBox::question(this, trClearDb, trRealyRemoveAllKeysTemplate_1S.arg(node->name()),
                              QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
      return;
    }

    node->removeAllKeys();
  }
}

void ExplorerTreeView::removeBranch() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerNSItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(sel);
  if (!node) {
    return;
  }

  int answer =
      QMessageBox::question(this, trRemoveBranch, trRemoveAllKeysTemplate_1S.arg(node->name()),
                            QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  node->removeBranch();
}

void ExplorerTreeView::setDefaultDb() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (node) {
    node->setDefault();
  }
}

void ExplorerTreeView::createKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  DbKeyDialog loadDb(trCreateKeyForDbTemplate_1S.arg(node->name()), server->Type(),
                     core::NDbKValue(), this);
  int result = loadDb.exec();
  if (result == QDialog::Accepted) {
    core::NDbKValue key = loadDb.key();
    node->createKey(key);
  }
}

void ExplorerTreeView::editKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
  }

  proxy::IServerSPtr server = node->server();
  DbKeyDialog loadDb(trEditKey_1S.arg(node->name()), server->Type(), node->dbv(), this);
  int result = loadDb.exec();
  if (result == QDialog::Accepted) {
    core::NDbKValue key = loadDb.key();
    node->editKey(key.Value());
  }
}

void ExplorerTreeView::viewKeys() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node =
      common::qt::item<common::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (!node) {
    return;
  }

  ViewKeysDialog diag(trViewKeyTemplate_1S.arg(node->name()), node->db(), this);
  diag.exec();
}

void ExplorerTreeView::loadValue() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
  }

  node->loadValueFromDb();
}

void ExplorerTreeView::renKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
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
    return;
  }

  QString new_key_name = reg_dialog.text();
  if (new_key_name.isEmpty()) {
    return;
  }

  if (new_key_name == name) {
    return;
  }

  node->renameKey(new_key_name);
}

void ExplorerTreeView::deleteKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
  }

  node->removeFromDb();
}

void ExplorerTreeView::watchKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
  }

  bool ok;
  QString name = node->name();
  int interval = QInputDialog::getInt(this, trSetIntervalOnKeyTemplate_1S.arg(name),
                                      trIntervalValue, 1000, 0, INT32_MAX, 1000, &ok);
  if (ok) {
    node->watchKey(interval);
  }
}

void ExplorerTreeView::setTTL() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (!node) {
    return;
  }

  bool ok;
  QString name = node->name();
  core::NKey key = node->key();
  int ttl = QInputDialog::getInt(this, trSetTTLOnKeyTemplate_1S.arg(name), trTTLValue, key.TTL(),
                                 -1, INT32_MAX, 100, &ok);
  if (ok) {
    node->setTTL(ttl);
  }
}

void ExplorerTreeView::startLoadDatabases(const proxy::events_info::LoadDatabasesInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabases(
    const proxy::events_info::LoadDatabasesInfoResponce& res) {
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

void ExplorerTreeView::startLoadDatabaseContent(
    const proxy::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabaseContent(
    const proxy::events_info::LoadDatabaseContentResponce& res) {
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
  if (proxy::SettingsManager::instance().FastViewKeys()) {
    loadValue();
  }

  QTreeView::mouseDoubleClickEvent(e);
}

void ExplorerTreeView::syncWithServer(proxy::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(connect(server, &proxy::IServer::LoadDatabasesStarted, this,
                 &ExplorerTreeView::startLoadDatabases));
  VERIFY(connect(server, &proxy::IServer::LoadDatabasesFinished, this,
                 &ExplorerTreeView::finishLoadDatabases));
  VERIFY(connect(server, &proxy::IServer::LoadDataBaseContentStarted, this,
                 &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(connect(server, &proxy::IServer::LoadDatabaseContentFinished, this,
                 &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(connect(server, &proxy::IServer::ExecuteStarted, this,
                 &ExplorerTreeView::startExecuteCommand));
  VERIFY(connect(server, &proxy::IServer::ExecuteFinished, this,
                 &ExplorerTreeView::finishExecuteCommand));

  VERIFY(connect(server, &proxy::IServer::FlushedDB, this, &ExplorerTreeView::flushDB));
  VERIFY(connect(server, &proxy::IServer::CurrentDataBaseChanged, this,
                 &ExplorerTreeView::currentDataBaseChange));
  VERIFY(connect(server, &proxy::IServer::KeyRemoved, this, &ExplorerTreeView::removeKey,
                 Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyAdded, this, &ExplorerTreeView::addKey,
                 Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyRenamed, this, &ExplorerTreeView::renameKey,
                 Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyLoaded, this, &ExplorerTreeView::loadKey,
                 Qt::DirectConnection));
  VERIFY(connect(server, &proxy::IServer::KeyTTLChanged, this, &ExplorerTreeView::changeTTLKey,
                 Qt::DirectConnection));
}

void ExplorerTreeView::unsyncWithServer(proxy::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(disconnect(server, &proxy::IServer::LoadDatabasesStarted, this,
                    &ExplorerTreeView::startLoadDatabases));
  VERIFY(disconnect(server, &proxy::IServer::LoadDatabasesFinished, this,
                    &ExplorerTreeView::finishLoadDatabases));
  VERIFY(disconnect(server, &proxy::IServer::LoadDataBaseContentStarted, this,
                    &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(disconnect(server, &proxy::IServer::LoadDatabaseContentFinished, this,
                    &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(disconnect(server, &proxy::IServer::ExecuteStarted, this,
                    &ExplorerTreeView::startExecuteCommand));
  VERIFY(disconnect(server, &proxy::IServer::ExecuteFinished, this,
                    &ExplorerTreeView::finishExecuteCommand));

  VERIFY(disconnect(server, &proxy::IServer::FlushedDB, this, &ExplorerTreeView::flushDB));
  VERIFY(disconnect(server, &proxy::IServer::CurrentDataBaseChanged, this,
                    &ExplorerTreeView::currentDataBaseChange));
  VERIFY(disconnect(server, &proxy::IServer::KeyRemoved, this, &ExplorerTreeView::removeKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyAdded, this, &ExplorerTreeView::addKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyRenamed, this, &ExplorerTreeView::renameKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyLoaded, this, &ExplorerTreeView::loadKey));
  VERIFY(disconnect(server, &proxy::IServer::KeyTTLChanged, this, &ExplorerTreeView::changeTTLKey));
}

void ExplorerTreeView::retranslateUi() {
  connectAction_->setText(trConnectDisconnect);
  openConsoleAction_->setText(translations::trOpenConsole);
  loadDatabaseAction_->setText(translations::trLoadDataBases);
  infoServerAction_->setText(translations::trInfo);
  propertyServerAction_->setText(translations::trProperty);
  setServerPassword_->setText(translations::trSetPassword);
  setMaxClientConnection_->setText(translations::trSetMaxNumberOfClients);
  historyServerAction_->setText(translations::trHistory);
  clearHistoryServerAction_->setText(translations::trClearHistory);
  closeServerAction_->setText(translations::trClose);
  closeClusterAction_->setText(translations::trClose);
  closeSentinelAction_->setText(translations::trClose);
  backupAction_->setText(translations::trBackup);
  importAction_->setText(translations::trImport);
  shutdownAction_->setText(translations::trShutdown);

  loadContentAction_->setText(translations::trLoadContOfDataBases);
  removeAllKeysAction_->setText(translations::trRemoveAllKeys);
  removeBranchAction_->setText(translations::trRemoveBranch);
  createKeyAction_->setText(translations::trCreateKey);
  editKeyAction_->setText(translations::trEdit);
  viewKeysAction_->setText(translations::trViewKeysDialog);
  pubSubAction_->setText(translations::trPubSubDialog);
  setDefaultDbAction_->setText(translations::trSetDefault);
  getValueAction_->setText(translations::trGetValue);
  renameKeyAction_->setText(trRenameKey);
  deleteKeyAction_->setText(translations::trDelete);
  watchKeyAction_->setText(translations::trWatch);
}

QModelIndex ExplorerTreeView::selectedIndex() const {
  QModelIndexList indexses = selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return proxy_model_->mapToSource(indexses[0]);
}

}  // namespace gui
}  // namespace fastonosql
