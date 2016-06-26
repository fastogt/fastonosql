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

#include <QAction>
#include <QHeaderView>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include "common/qt/convert_string.h"
#include "common/logger.h"

#include "core/settings_manager.h"
#include "core/icluster.h"
#include "core/isentinel.h"

#include "translations/global.h"

#include "gui/explorer/explorer_tree_model.h"
#include "gui/dialogs/info_server_dialog.h"
#include "gui/dialogs/property_server_dialog.h"
#include "gui/dialogs/history_server_dialog.h"
#include "gui/dialogs/load_contentdb_dialog.h"
#include "gui/dialogs/create_dbkey_dialog.h"
#include "gui/dialogs/view_keys_dialog.h"
#include "gui/dialogs/change_password_server_dialog.h"

namespace {
  const QString trCreateKeyForDbTemplate_1S = QObject::tr("Create key for %1 database");
  const QString trRemoveBranch = QObject::tr("Remove branch");
  const QString trRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from branch %1?");
  const QString trViewKeyTemplate_1S = QObject::tr("View key in %1 database");
  const QString trConnectDisconnect = QObject::tr("Connect/Disconnect");
  const QString trClearDb = QObject::tr("Clear database");
  const QString trRealyRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from %1 database?");
  const QString trLoadContentTemplate_1S = QObject::tr("Load %1 content");
  const QString trReallyShutdownTemplate_1S = QObject::tr("Really shutdown \"%1\" server?");
  const QString trSetMaxConnectionOnServerTemplate_1S = QObject::tr("Set max connection on %1 server");
  const QString trMaximumConnectionTemplate = QObject::tr("Maximum connection:");
  const QString trChangePasswordTemplate_1S = QObject::tr("Change password for %1 server");
}

namespace fastonosql {
namespace gui {

ExplorerTreeView::ExplorerTreeView(QWidget* parent)
  : QTreeView(parent) {
  setModel(new ExplorerTreeModel(this));

  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
                 this, SLOT(showContextMenu(const QPoint&))));

  connectAction_ = new QAction(this);
  VERIFY(connect(connectAction_, &QAction::triggered,
                 this, &ExplorerTreeView::connectDisconnectToServer));
  openConsoleAction_ = new QAction(this);
  VERIFY(connect(openConsoleAction_, &QAction::triggered, this, &ExplorerTreeView::openConsole));
  loadDatabaseAction_ = new QAction(this);
  VERIFY(connect(loadDatabaseAction_, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

  infoServerAction_ = new QAction(this);
  VERIFY(connect(infoServerAction_, &QAction::triggered,
                 this, &ExplorerTreeView::openInfoServerDialog));

  propertyServerAction_ = new QAction(this);
  VERIFY(connect(propertyServerAction_, &QAction::triggered,
                 this, &ExplorerTreeView::openPropertyServerDialog));

  setServerPassword_ = new QAction(this);
  VERIFY(connect(setServerPassword_, &QAction::triggered,
                 this, &ExplorerTreeView::openSetPasswordServerDialog));

  setMaxClientConnection_ = new QAction(this);
  VERIFY(connect(setMaxClientConnection_, &QAction::triggered,
                 this, &ExplorerTreeView::openMaxClientSetDialog));

  historyServerAction_ = new QAction(this);
  VERIFY(connect(historyServerAction_, &QAction::triggered,
                 this, &ExplorerTreeView::openHistoryServerDialog));

  clearHistoryServerAction_ = new QAction(this);
  VERIFY(connect(clearHistoryServerAction_, &QAction::triggered,
                 this, &ExplorerTreeView::clearHistory));

  closeServerAction_ = new QAction(this);
  VERIFY(connect(closeServerAction_, &QAction::triggered,
                 this, &ExplorerTreeView::closeServerConnection));

  closeClusterAction_ = new QAction(this);
  VERIFY(connect(closeClusterAction_, &QAction::triggered,
                 this, &ExplorerTreeView::closeClusterConnection));

  closeSentinelAction_ = new QAction(this);
  VERIFY(connect(closeSentinelAction_, &QAction::triggered,
                 this, &ExplorerTreeView::closeSentinelConnection));

  importAction_ = new QAction(this);
  VERIFY(connect(importAction_, &QAction::triggered, this, &ExplorerTreeView::importServer));

  backupAction_ = new QAction(this);
  VERIFY(connect(backupAction_, &QAction::triggered, this, &ExplorerTreeView::backupServer));

  shutdownAction_ = new QAction(this);
  VERIFY(connect(shutdownAction_, &QAction::triggered, this, &ExplorerTreeView::shutdownServer));

  loadContentAction_ = new QAction(this);
  VERIFY(connect(loadContentAction_, &QAction::triggered, this, &ExplorerTreeView::loadContentDb));

  removeAllKeysAction_ = new QAction(this);
  VERIFY(connect(removeAllKeysAction_, &QAction::triggered, this, &ExplorerTreeView::removeAllKeys));

  removeBranchAction_ = new QAction(this);
  VERIFY(connect(removeBranchAction_, &QAction::triggered, this, &ExplorerTreeView::removeBranch));

  setDefaultDbAction_ = new QAction(this);
  VERIFY(connect(setDefaultDbAction_, &QAction::triggered, this, &ExplorerTreeView::setDefaultDb));

  createKeyAction_ = new QAction(this);
  VERIFY(connect(createKeyAction_, &QAction::triggered, this, &ExplorerTreeView::createKey));

  viewKeysAction_ = new QAction(this);
  VERIFY(connect(viewKeysAction_, &QAction::triggered, this, &ExplorerTreeView::viewKeys));

  getValueAction_ = new QAction(this);
  VERIFY(connect(getValueAction_, &QAction::triggered, this, &ExplorerTreeView::getValue));

  deleteKeyAction_ = new QAction(this);
  VERIFY(connect(deleteKeyAction_, &QAction::triggered, this, &ExplorerTreeView::deleteKey));

  retranslateUi();
}

void ExplorerTreeView::addServer(core::IServerSPtr server) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  if (!mod) {
    DNOTREACHED();
    return;
  }

  syncWithServer(server.get());

  mod->addServer(server);
}

void ExplorerTreeView::removeServer(core::IServerSPtr server) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  if (!mod) {
    DNOTREACHED();
    return;
  }

  unsyncWithServer(server.get());

  mod->removeServer(server);
  emit closeServer(server);
}

void ExplorerTreeView::addSentinel(core::ISentinelSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  core::ISentinel::sentinels_t nodes = sentinel->sentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    core::Sentinel sent = nodes[i];
    syncWithServer(sent.sentinel.get());
    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      syncWithServer(sent.sentinels_nodes[j].get());
    }
  }

  mod->addSentinel(sentinel);
}

void ExplorerTreeView::removeSentinel(core::ISentinelSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  core::ISentinel::sentinels_t nodes = sentinel->sentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    core::Sentinel sent = nodes[i];
    unsyncWithServer(sent.sentinel.get());
    for (size_t j = 0; j < sent.sentinels_nodes.size(); ++j) {
      unsyncWithServer(sent.sentinels_nodes[j].get());
    }
  }

  mod->removeSentinel(sentinel);
  emit closeSentinel(sentinel);
}

void ExplorerTreeView::addCluster(core::IClusterSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  auto nodes = cluster->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    syncWithServer(nodes[i].get());
  }

  mod->addCluster(cluster);
}

void ExplorerTreeView::removeCluster(core::IClusterSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  ExplorerTreeModel* mod = static_cast<ExplorerTreeModel*>(model());
  auto nodes = cluster->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    unsyncWithServer(nodes[i].get());
  }

  mod->removeCluster(cluster);
  emit closeCluster(cluster);
}

void ExplorerTreeView::showContextMenu(const QPoint& point) {
  QPoint menuPoint = mapToGlobal(point);
  menuPoint.setY(menuPoint.y() + header()->height());

  QModelIndex sel = selectedIndex();
  if (sel.isValid()) {
    IExplorerTreeItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, IExplorerTreeItem*>(sel);
    if (!node) {
      DNOTREACHED();
      return;
    }

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
      core::IServerSPtr server = server_node->server();
      bool isCon = server->isConnected();
      bool isAuth = server->isAuthenticated();
      bool isRedis = server->type() == core::REDIS;

      bool isClusterMember = dynamic_cast<ExplorerClusterItem*>(node->parent()) != nullptr;  // +

      loadDatabaseAction_->setEnabled(isAuth);
      menu.addAction(loadDatabaseAction_);
      infoServerAction_->setEnabled(isAuth);
      menu.addAction(infoServerAction_);
      propertyServerAction_->setEnabled(isAuth && isRedis);
      menu.addAction(propertyServerAction_);

      setServerPassword_->setEnabled(isAuth && isRedis);
      menu.addAction(setServerPassword_);

      setMaxClientConnection_->setEnabled(isAuth && isRedis);
      menu.addAction(setMaxClientConnection_);

      menu.addAction(historyServerAction_);
      menu.addAction(clearHistoryServerAction_);
      closeServerAction_->setEnabled(!isClusterMember);
      menu.addAction(closeServerAction_);

      bool isCanRemote = server->isCanRemote();
      bool isLocal = true;
      if (isCanRemote) {
        core::IServerRemote* rserver = dynamic_cast<core::IServerRemote*>(server.get());  // +
        CHECK(rserver);
        common::net::hostAndPort host = rserver->host();
        isLocal = host.isLocalHost();
      }

      importAction_->setEnabled(!isCon && isLocal && isRedis);
      menu.addAction(importAction_);
      backupAction_->setEnabled(isCon && isLocal && isRedis);
      menu.addAction(backupAction_);
      shutdownAction_->setEnabled(isAuth && isRedis);
      menu.addAction(shutdownAction_);

      menu.exec(menuPoint);
    } else if (node->type() == IExplorerTreeItem::eDatabase) {
      ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);

      QMenu menu(this);
      menu.addAction(loadContentAction_);
      bool isDefault = db->isDefault();
      core::IServerSPtr server = db->server();

      bool isCon = server->isConnected();
      loadContentAction_->setEnabled(isDefault && isCon);

      menu.addAction(createKeyAction_);
      createKeyAction_->setEnabled(isDefault && isCon);

      menu.addAction(viewKeysAction_);
      viewKeysAction_->setEnabled(isDefault && isCon);

      menu.addAction(removeAllKeysAction_);
      removeAllKeysAction_->setEnabled(isDefault && isCon);

      menu.addAction(setDefaultDbAction_);
      setDefaultDbAction_->setEnabled(!isDefault && isCon);
      menu.exec(menuPoint);
    } else if (node->type() == IExplorerTreeItem::eNamespace) {
      ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);

      QMenu menu(this);
      core::IServerSPtr server = ns->server();
      ExplorerDatabaseItem* db = ns->db();
      bool isDefault = db && db->isDefault();
      bool isCon = server->isConnected();

      menu.addAction(removeBranchAction_);
      removeBranchAction_->setEnabled(isDefault && isCon);
      menu.exec(menuPoint);
    } else if (node->type() == IExplorerTreeItem::eKey) {
      ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);

      QMenu menu(this);
      core::IServerSPtr server = key->server();

      bool isCon = server->isConnected();
      menu.addAction(getValueAction_);
      getValueAction_->setEnabled(isCon);

      menu.addAction(deleteKeyAction_);
      deleteKeyAction_->setEnabled(isCon);
      menu.exec(menuPoint);
    }
  }
}

void ExplorerTreeView::connectDisconnectToServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  if (server->isConnected()) {
    core::events_info::DisConnectInfoRequest req(this);
    server->disconnect(req);
  } else {
    core::events_info::ConnectInfoRequest req(this);
    server->connect(req);
  }
}

void ExplorerTreeView::openConsole() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (node) {
    emit openedConsole(node->server(), QString());
  }
}

void ExplorerTreeView::loadDatabases() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (node) {
    node->loadDatabases();
  }
}

void ExplorerTreeView::openInfoServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
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

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
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

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  ChangePasswordServerDialog pass(trChangePasswordTemplate_1S.arg(node->name()),
                                  server, this);
  pass.exec();
}

void ExplorerTreeView::openMaxClientSetDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  bool ok;
  QString name = common::ConvertFromString<QString>(server->name());
  int maxcl = QInputDialog::getInt(this, trSetMaxConnectionOnServerTemplate_1S.arg(name),
                                         trMaximumConnectionTemplate, 10000, 1, INT32_MAX, 100, &ok);
  if (ok) {
    core::events_info::ChangeMaxConnectionRequest req(this, maxcl);
    server->setMaxConnection(req);
  }
}

void ExplorerTreeView::openHistoryServerDialog() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
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

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  if (!server) {
    return;
  }

  core::events_info::ClearServerHistoryRequest req(this);
  server->clearHistory(req);
}

void ExplorerTreeView::closeServerConnection() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* snode = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (snode) {
    core::IServerSPtr server = snode->server();
    if (server) {
      removeServer(server);
    }
    return;
  }

  ExplorerClusterItem* cnode = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerClusterItem*>(sel);
  if (cnode && cnode->type() == IExplorerTreeItem::eCluster) {
    core::IClusterSPtr server = cnode->cluster();
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

  ExplorerClusterItem* cnode = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerClusterItem*>(sel);
  if (!cnode) {
    return;
  }

  core::IClusterSPtr server = cnode->cluster();
  if (server) {
    removeCluster(server);
  }
}

void ExplorerTreeView::closeSentinelConnection() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerSentinelItem* snode = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerSentinelItem*>(sel);
  if (!snode) {
    return;
  }

  core::ISentinelSPtr sent = snode->sentinel();
  if (sent) {
    removeSentinel(sent);
  }
}

void ExplorerTreeView::backupServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  QString filepath = QFileDialog::getOpenFileName(this, translations::trBackup,
                                                  QString(), translations::trfilterForRdb);
  if (!filepath.isEmpty() && server) {
    core::events_info::BackupInfoRequest req(this, common::ConvertToString(filepath));
    server->backupToPath(req);
  }
}

void ExplorerTreeView::importServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  QString filepath = QFileDialog::getOpenFileName(this, translations::trImport,
                                                  QString(), translations::trfilterForRdb);
  if (filepath.isEmpty() && server) {
    core::events_info::ExportInfoRequest req(this, common::ConvertToString(filepath));
    server->exportFromPath(req);
  }
}

void ExplorerTreeView::shutdownServer() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerServerItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerServerItem*>(sel);
  if (!node) {
    return;
  }

  core::IServerSPtr server = node->server();
  if (server && server->isConnected()) {
    // Ask user
    QString name = common::ConvertFromString<QString>(server->name());
    int answer = QMessageBox::question(this, translations::trShutdown, trReallyShutdownTemplate_1S.arg(name),
                                       QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
      return;
    }

    core::events_info::ShutDownInfoRequest req(this);
    server->shutDown(req);
  }
}

void ExplorerTreeView::loadContentDb() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (!node) {
    return;
  }

  LoadContentDbDialog loadDb(trLoadContentTemplate_1S.arg(node->name()), node->server()->type(), this);
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

  ExplorerDatabaseItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (node) {
    int answer = QMessageBox::question(this, trClearDb,
                                       trRealyRemoveAllKeysTemplate_1S.arg(node->name()),
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

  ExplorerNSItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerNSItem*>(sel);
  if (node) {
    return;
  }

  int answer = QMessageBox::question(this, trRemoveBranch, trRemoveAllKeysTemplate_1S.arg(node->name()),
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

  ExplorerDatabaseItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (node) {
    node->setDefault();
  }
}

void ExplorerTreeView::createKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (!node) {
    return;
  }

  CreateDbKeyDialog loadDb(trCreateKeyForDbTemplate_1S.arg(node->name()), node->server()->type(), this);
  int result = loadDb.exec();
  if (result == QDialog::Accepted) {
    core::NDbKValue key = loadDb.key();
    node->createKey(key);
  }
}

void ExplorerTreeView::viewKeys() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerDatabaseItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerDatabaseItem*>(sel);
  if (node) {
    ViewKeysDialog diag(trViewKeyTemplate_1S.arg(node->name()), node->db(), this);
    diag.exec();
  }
}

void ExplorerTreeView::getValue() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (node) {
    node->loadValueFromDb();
  }
}

void ExplorerTreeView::deleteKey() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ExplorerKeyItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, ExplorerKeyItem*>(sel);
  if (node) {
    node->removeFromDb();
  }
}

void ExplorerTreeView::startLoadDatabases(const core::events_info::LoadDatabasesInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabases(const core::events_info::LoadDatabasesInfoResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServer* serv = qobject_cast<core::IServer*>(sender());
  CHECK(serv);

  ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
  CHECK(mod);

  core::events_info::LoadDatabasesInfoResponce::database_info_cont_type dbs = res.databases;
  for (size_t i = 0; i < dbs.size(); ++i) {
    core::IDataBaseInfoSPtr db = dbs[i];
    mod->addDatabase(serv, db);
  }
}

void ExplorerTreeView::startSetDefaultDatabase(const core::events_info::SetDefaultDatabaseRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishSetDefaultDatabase(const core::events_info::SetDefaultDatabaseResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServer* serv = qobject_cast<core::IServer*>(sender());
  CHECK(serv);

  core::IDataBaseInfoSPtr db = res.inf;
  ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
  CHECK(mod);
  mod->setDefaultDb(serv, db);
}

void ExplorerTreeView::startLoadDatabaseContent(const core::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabaseContent(const core::events_info::LoadDatabaseContentResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServer* serv = qobject_cast<core::IServer*>(sender());
  CHECK(serv);

  ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
  CHECK(mod);

  core::events_info::LoadDatabaseContentResponce::keys_container_t keys = res.keys;
  std::string ns = serv->nsSeparator();
  for (size_t i = 0; i < keys.size(); ++i) {
    core::NDbKValue key = keys[i];
    mod->addKey(serv, res.inf, key, ns);
  }

  mod->updateDb(serv, res.inf);
}

void ExplorerTreeView::startClearDatabase(const core::events_info::ClearDatabaseRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishClearDatabase(const core::events_info::ClearDatabaseResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServer* serv = qobject_cast<core::IServer*>(sender());
  CHECK(serv);

  ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
  CHECK(mod);

  mod->removeAllKeys(serv, res.inf);
}

void ExplorerTreeView::startExecuteCommand(const core::events_info::CommandRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishExecuteCommand(const core::events_info::CommandResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServer* serv = qobject_cast<core::IServer*>(sender());
  CHECK(serv);

  ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
  CHECK(mod);

  std::string ns = serv->nsSeparator();
  core::CommandKeySPtr key = res.cmd;
  core::NDbKValue dbv = key->key();
  if (key->type() == core::CommandKey::C_DELETE) {
    mod->removeKey(serv, res.inf, dbv);
  } else if (key->type() == core::CommandKey::C_CREATE) {
    mod->addKey(serv, res.inf, dbv, ns);
  }
}

void ExplorerTreeView::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QTreeView::changeEvent(e);
}

void ExplorerTreeView::mouseDoubleClickEvent(QMouseEvent* e) {
  if (core::SettingsManager::instance().fastViewKeys()) {
    getValue();
  }

  QTreeView::mouseDoubleClickEvent(e);
}

void ExplorerTreeView::syncWithServer(core::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(connect(server, &core::IServer::startedLoadDatabases,
                 this, &ExplorerTreeView::startLoadDatabases));
  VERIFY(connect(server, &core::IServer::finishedLoadDatabases,
                 this, &ExplorerTreeView::finishLoadDatabases));
  VERIFY(connect(server, &core::IServer::startedSetDefaultDatabase,
                 this, &ExplorerTreeView::startSetDefaultDatabase));
  VERIFY(connect(server, &core::IServer::finishedSetDefaultDatabase,
                 this, &ExplorerTreeView::finishSetDefaultDatabase));
  VERIFY(connect(server, &core::IServer::startedLoadDataBaseContent,
                 this, &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(connect(server, &core::IServer::finishedLoadDatabaseContent,
                 this, &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(connect(server, &core::IServer::startedClearDatabase,
                 this, &ExplorerTreeView::startClearDatabase));
  VERIFY(connect(server, &core::IServer::finishedClearDatabase,
                 this, &ExplorerTreeView::finishClearDatabase));
  VERIFY(connect(server, &core::IServer::startedExecuteCommand,
                 this, &ExplorerTreeView::startExecuteCommand));
  VERIFY(connect(server, &core::IServer::finishedExecuteCommand,
                 this, &ExplorerTreeView::finishExecuteCommand));
}

void ExplorerTreeView::unsyncWithServer(core::IServer* server) {
  if (!server) {
    return;
  }

  VERIFY(disconnect(server, &core::IServer::startedLoadDatabases,
                    this, &ExplorerTreeView::startLoadDatabases));
  VERIFY(disconnect(server, &core::IServer::finishedLoadDatabases,
                    this, &ExplorerTreeView::finishLoadDatabases));
  VERIFY(disconnect(server, &core::IServer::startedSetDefaultDatabase,
                    this, &ExplorerTreeView::startSetDefaultDatabase));
  VERIFY(disconnect(server, &core::IServer::finishedSetDefaultDatabase,
                    this, &ExplorerTreeView::finishSetDefaultDatabase));
  VERIFY(disconnect(server, &core::IServer::startedLoadDataBaseContent,
                    this, &ExplorerTreeView::startLoadDatabaseContent));
  VERIFY(disconnect(server, &core::IServer::finishedLoadDatabaseContent,
                    this, &ExplorerTreeView::finishLoadDatabaseContent));
  VERIFY(disconnect(server, &core::IServer::startedExecuteCommand,
                    this, &ExplorerTreeView::startExecuteCommand));
  VERIFY(disconnect(server, &core::IServer::finishedExecuteCommand,
                    this, &ExplorerTreeView::finishExecuteCommand));
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
  viewKeysAction_->setText(translations::trViewKeysDialog);
  setDefaultDbAction_->setText(translations::trSetDefault);
  getValueAction_->setText(translations::trValue);
  deleteKeyAction_->setText(translations::trDelete);
}

QModelIndex ExplorerTreeView::selectedIndex() const {
  QModelIndexList indexses = selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return indexses[0];
}

}  // namespace gui
}  // namespace fastonosql
