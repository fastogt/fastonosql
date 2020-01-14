/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/explorer/explorer_tree_view.h"

#include <string>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>

#include <common/convert2string.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include <common/qt/gui/regexp_input_dialog.h>

#include "proxy/cluster/icluster.h"
#include "proxy/sentinel/isentinel.h"
#include "proxy/server/iserver_remote.h"

#include "gui/dialogs/clients_monitor_dialog.h"
#include "gui/dialogs/dbkey_dialog.h"
#include "gui/dialogs/history_server_dialog.h"
#include "gui/dialogs/info_server_dialog.h"
#include "gui/dialogs/load_contentdb_dialog.h"
#include "gui/dialogs/property_server_dialog.h"
#include "gui/dialogs/pub_sub_dialog.h"
#include "gui/dialogs/view_keys_dialog.h"

#include "gui/gui_factory.h"
#include "gui/models/explorer_tree_model.h"
#include "gui/models/explorer_tree_sort_filter_proxy_model.h"
#include "gui/models/items/explorer_tree_item.h"

#include "translations/global.h"

namespace {
const QString trRemoveDatabaseTemplate_1S = QObject::tr("Really remove database %1?");
const QString trRealyRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from %1 database?");

const QString trCreateKeyForDbTemplate_1S = QObject::tr("Create key for %1 database");
const QString trEditKey_1S = QObject::tr("Edit key %1");
const QString trRemoveAllKeysTemplate_1S = QObject::tr("Really remove all keys from branch %1?");
const QString trViewKeyTemplate_1S = QObject::tr("View keys in %1 database");
const QString trViewChannelsTemplate_1S = QObject::tr("View channels in %1 server");
const QString trViewClientsTemplate_1S = QObject::tr("View clients in %1 server");
const QString trClearDb = QObject::tr("Clear database");
const QString trLoadContentTemplate_1S = QObject::tr("Load keys in %1 database");
const QString trSetMaxConnectionOnServerTemplate_1S = QObject::tr("Set max connection on %1 server");
const QString trSetTTLOnKeyTemplate_1S = QObject::tr("Set ttl for %1 key");
const QString trNewTTLSeconds = QObject::tr("New TTL in seconds:");
const QString trSetIntervalOnKeyTemplate_1S = QObject::tr("Set watch interval for %1 key");
const QString trIntervalValue = QObject::tr("Interval msec:");
const QString trSetTTL = QObject::tr("Set TTL");
const QString trRemoveTTL = QObject::tr("Remove TTL");
const QString trRenameKeyLabel = QObject::tr("New branch name:");
const QString trRenameBranchLabel = QObject::tr("New branch name:");
const QString trCreateDatabase_1S = QObject::tr("Create database on %1 server");
const QString trPropertiesTemplate_1S = QObject::tr("%1 properties");
const QString trHistoryTemplate_1S = QObject::tr("%1 history");
const QString trCopyToClipboard = QObject::tr("Copy to clipboard");
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

  setMinimumSize(QSize(min_width, min_height));
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

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
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
#endif

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
  IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(index);
  if (!node) {
    DNOTREACHED();
    return;
  }

  QPoint menu_point = mapToGlobal(point);
  menu_point.setY(menu_point.y() + header()->height());
  if (node->type() == IExplorerTreeItem::eServer) {
    ExplorerServerItem* server_node = static_cast<ExplorerServerItem*>(node);

    proxy::IServerSPtr server = server_node->server();
    const bool is_connected = server->IsConnected();
    const bool is_redis = server->GetType() == core::REDIS;

    QMenu menu(this);
    QAction* connect_action = new QAction(is_connected ? translations::trDisconnect : translations::trConnect, this);
    VERIFY(connect(connect_action, &QAction::triggered, this, &ExplorerTreeView::connectDisconnectToServer));

    QAction* open_console_action = new QAction(translations::trOpenConsole, this);
    VERIFY(connect(open_console_action, &QAction::triggered, this, &ExplorerTreeView::openConsole));

    menu.addAction(connect_action);
    menu.addAction(open_console_action);

    QAction* load_database_action = new QAction(translations::trLoadDataBases, this);
    VERIFY(connect(load_database_action, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

    QAction* info_server_action = new QAction(translations::trInfo, this);
    VERIFY(connect(info_server_action, &QAction::triggered, this, &ExplorerTreeView::openInfoServerDialog));

    load_database_action->setEnabled(is_connected);
    menu.addAction(load_database_action);

    if (server->IsCanCreateDatabase()) {
      QAction* createDatabaseAction = new QAction(translations::trCreateDatabase, this);
      VERIFY(connect(createDatabaseAction, &QAction::triggered, this, &ExplorerTreeView::createDb));
      createDatabaseAction->setEnabled(is_connected);
      menu.addAction(createDatabaseAction);
    }

    info_server_action->setEnabled(is_connected);
    menu.addAction(info_server_action);

    if (is_redis) {
      QAction* property_server_action = new QAction(translations::trProperty, this);
      VERIFY(connect(property_server_action, &QAction::triggered, this, &ExplorerTreeView::openPropertyServerDialog));

      QAction* pub_sub_action = new QAction(translations::trPublishSubscribe, this);
      VERIFY(connect(pub_sub_action, &QAction::triggered, this, &ExplorerTreeView::viewPubSub));

      QAction* clients_monitor_action = new QAction(translations::trClientsMonitor, this);
      VERIFY(connect(clients_monitor_action, &QAction::triggered, this, &ExplorerTreeView::viewClientsMonitor));

      property_server_action->setEnabled(is_connected);
      menu.addAction(property_server_action);

      pub_sub_action->setEnabled(is_connected);
      menu.addAction(pub_sub_action);

      clients_monitor_action->setEnabled(is_connected);
      menu.addAction(clients_monitor_action);

      bool is_local = true;
      bool is_can_remote = server->IsCanRemote();
      if (is_can_remote) {
        proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server.get());  // +
        if (rserver) {
          common::net::HostAndPort host = rserver->GetHost();
          is_local = host.IsLocalHost();  // failed if ssh connection
        }
      }

      QAction* export_action = new QAction(translations::trBackup, this);
      VERIFY(connect(export_action, &QAction::triggered, this, &ExplorerTreeView::exportServer));

      QAction* import_action = new QAction(translations::trRestore, this);
      VERIFY(connect(import_action, &QAction::triggered, this, &ExplorerTreeView::importServer));

      export_action->setEnabled(!is_connected && is_local);
      menu.addAction(export_action);
      import_action->setEnabled(is_connected && is_local);
      menu.addAction(import_action);
    }

    QAction* history_server_action = new QAction(translations::trHistory, this);
    VERIFY(connect(history_server_action, &QAction::triggered, this, &ExplorerTreeView::openHistoryServerDialog));

    QAction* clear_history_server_action = new QAction(translations::trClearHistory, this);
    VERIFY(connect(clear_history_server_action, &QAction::triggered, this, &ExplorerTreeView::clearHistory));

    QAction* close_server_action = new QAction(translations::trClose, this);
    VERIFY(connect(close_server_action, &QAction::triggered, this, &ExplorerTreeView::closeServerConnection));

    menu.addAction(history_server_action);
    menu.addAction(clear_history_server_action);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
    common::qt::gui::TreeItem* par = node->parent();
    bool is_cluster_member = dynamic_cast<ExplorerClusterItem*>(par) != nullptr;  // +
    close_server_action->setEnabled(!is_cluster_member);
#endif
    menu.addAction(close_server_action);

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  } else if (node->type() == IExplorerTreeItem::eDatabase) {
    ExplorerDatabaseItem* db = static_cast<ExplorerDatabaseItem*>(node);

    QMenu menu(this);
    QAction* load_content_action = new QAction(translations::trLoadContOfDataBases, this);
    VERIFY(connect(load_content_action, &QAction::triggered, this, &ExplorerTreeView::loadContentDb));

    QAction* create_key_action = new QAction(translations::trCreateKey, this);
    VERIFY(connect(create_key_action, &QAction::triggered, this, &ExplorerTreeView::createKey));

    QAction* view_keys_action = new QAction(translations::trViewKeys, this);
    VERIFY(connect(view_keys_action, &QAction::triggered, this, &ExplorerTreeView::viewKeys));

    QAction* remove_all_keys_action = new QAction(translations::trRemoveAllKeys, this);
    VERIFY(connect(remove_all_keys_action, &QAction::triggered, this, &ExplorerTreeView::removeAllKeys));

    QAction* set_default_db_action = new QAction(translations::trSetDefault, this);
    VERIFY(connect(set_default_db_action, &QAction::triggered, this, &ExplorerTreeView::setDefaultDb));

    menu.addAction(load_content_action);
    bool is_default = db->isDefault();
    proxy::IServerSPtr server = db->server();

    bool is_connected = server->IsConnected();
    load_content_action->setEnabled(is_default && is_connected);

    menu.addAction(create_key_action);
    create_key_action->setEnabled(is_default && is_connected);

    menu.addAction(view_keys_action);
    view_keys_action->setEnabled(is_default && is_connected);

    menu.addAction(remove_all_keys_action);
    remove_all_keys_action->setEnabled(is_default && is_connected);

    menu.addAction(set_default_db_action);
    set_default_db_action->setEnabled(!is_default && is_connected);

    if (server->IsCanRemoveDatabase()) {
      QAction* remove_database_action = new QAction(translations::trRemove, this);
      VERIFY(connect(remove_database_action, &QAction::triggered, this, &ExplorerTreeView::removeDb));
      remove_database_action->setEnabled(!is_default && is_connected);
      menu.addAction(remove_database_action);
    }

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  } else if (node->type() == IExplorerTreeItem::eNamespace) {
    ExplorerNSItem* ns = static_cast<ExplorerNSItem*>(node);

    QMenu menu(this);

    QAction* addKeyToBranchAction = new QAction(translations::trCreateKey, this);
    VERIFY(connect(addKeyToBranchAction, &QAction::triggered, this, &ExplorerTreeView::addKeyToBranch));

    QAction* renameBranchAction = new QAction(translations::trRename, this);
    VERIFY(connect(renameBranchAction, &QAction::triggered, this, &ExplorerTreeView::renameBranch));

    QAction* removeBranchAction = new QAction(translations::trRemove, this);
    VERIFY(connect(removeBranchAction, &QAction::triggered, this, &ExplorerTreeView::remBranch));

    proxy::IServerSPtr server = ns->server();
    ExplorerDatabaseItem* db = ns->db();
    bool is_default = db && db->isDefault();
    bool is_connected = server->IsConnected();

    menu.addAction(addKeyToBranchAction);
    addKeyToBranchAction->setEnabled(is_default && is_connected);
    menu.addAction(renameBranchAction);
    renameBranchAction->setEnabled(is_default && is_connected);
    menu.addAction(removeBranchAction);
    removeBranchAction->setEnabled(is_default && is_connected);

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  } else if (node->type() == IExplorerTreeItem::eKey) {
    ExplorerKeyItem* key = static_cast<ExplorerKeyItem*>(node);

    QMenu menu(this);
    QAction* get_value_action = new QAction(translations::trGetValue, this);
    VERIFY(connect(get_value_action, &QAction::triggered, this, &ExplorerTreeView::loadValue));

    QAction* get_type_action = new QAction(translations::trGetType, this);
    VERIFY(connect(get_type_action, &QAction::triggered, this, &ExplorerTreeView::loadType));

    QAction* edit_key_action = new QAction(translations::trEditValue, this);
    VERIFY(connect(edit_key_action, &QAction::triggered, this, &ExplorerTreeView::editKey));

    QAction* rename_key_action = new QAction(translations::trRename, this);
    VERIFY(connect(rename_key_action, &QAction::triggered, this, &ExplorerTreeView::renKey));

    QAction* delete_key_action = new QAction(translations::trRemove, this);
    VERIFY(connect(delete_key_action, &QAction::triggered, this, &ExplorerTreeView::remKey));

    QAction* watch_key_action = new QAction(translations::trWatch, this);
    VERIFY(connect(watch_key_action, &QAction::triggered, this, &ExplorerTreeView::watchKey));

    proxy::IServerSPtr server = key->server();

    bool is_connected = server->IsConnected();
    menu.addAction(get_value_action);
    get_value_action->setEnabled(is_connected);
    menu.addAction(get_type_action);
    get_type_action->setEnabled(is_connected);
    bool is_ttl_supported = server->IsSupportTTLKeys();
    if (is_ttl_supported) {
      QAction* set_ttl_key_action = new QAction(trSetTTL, this);
      set_ttl_key_action->setEnabled(is_connected);
      VERIFY(connect(set_ttl_key_action, &QAction::triggered, this, &ExplorerTreeView::setTTL));
      menu.addAction(set_ttl_key_action);

      QAction* remove_ttl_key_action = new QAction(trRemoveTTL, this);
      remove_ttl_key_action->setEnabled(is_connected);
      VERIFY(connect(remove_ttl_key_action, &QAction::triggered, this, &ExplorerTreeView::removeTTL));
      menu.addAction(remove_ttl_key_action);
    }
    menu.addAction(rename_key_action);
    rename_key_action->setEnabled(is_connected);
    menu.addAction(edit_key_action);
    edit_key_action->setEnabled(is_connected);
    menu.addAction(delete_key_action);
    delete_key_action->setEnabled(is_connected);
    menu.addAction(watch_key_action);
    watch_key_action->setEnabled(is_connected);

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  }
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  else if (node->type() == IExplorerTreeItem::eCluster) {
    QMenu menu(this);
    QAction* close_cluster_action = new QAction(translations::trClose, this);
    VERIFY(connect(close_cluster_action, &QAction::triggered, this, &ExplorerTreeView::closeClusterConnection));
    menu.addAction(close_cluster_action);

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  } else if (node->type() == IExplorerTreeItem::eSentinel) {
    QMenu menu(this);
    QAction* close_sentinel_action = new QAction(translations::trClose, this);
    VERIFY(connect(close_sentinel_action, &QAction::triggered, this, &ExplorerTreeView::closeSentinelConnection));
    menu.addAction(close_sentinel_action);

    QAction* copy_to_clipboard_action = new QAction(trCopyToClipboard, this);
    VERIFY(connect(copy_to_clipboard_action, &QAction::triggered, this, &ExplorerTreeView::copyToClipboard));
    menu.addAction(copy_to_clipboard_action);
    menu.exec(menu_point);
  }
#endif
}

void ExplorerTreeView::copyToClipboard() {
  QString buffer;
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (int i = 0; i < selected.size(); ++i) {
    IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(selected[i]);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    buffer += node->name();
    if (i != selected.size() - 1) {
      buffer += END_COMMAND_CHAR;
    }
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(buffer);
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

    QString server_name;
    common::ConvertFromString(server->GetName(), &server_name);
    auto infDialog = createDialog<InfoServerDialog>(QObject::tr("%1 info").arg(server_name), server, this);  // +
    infDialog->exec();
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

    QString server_name;
    common::ConvertFromString(server->GetName(), &server_name);
    const QIcon dialog_icon = gui::GuiFactory::GetInstance().icon(node->server()->GetType());
    auto dialog =
        createDialog<PropertyServerDialog>(trPropertiesTemplate_1S.arg(server_name), dialog_icon, server, this);  // +
    dialog->exec();
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

    QString server_name;
    common::ConvertFromString(server->GetName(), &server_name);
    const QIcon dialog_icon = GuiFactory::GetInstance().icon(server->GetType());
    auto histDialog =
        createDialog<ServerHistoryDialog>(trHistoryTemplate_1S.arg(server_name), dialog_icon, server, this);  // +
    histDialog->exec();
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
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
    ExplorerClusterItem* cnode = common::qt::item<common::qt::gui::TreeItem*, ExplorerClusterItem*>(ind);
    if (cnode && cnode->type() == IExplorerTreeItem::eCluster) {
      proxy::IClusterSPtr server = cnode->cluster();
      if (server) {
        removeCluster(server);
      }
      continue;
    }
#endif
  }
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
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
#endif

void ExplorerTreeView::viewPubSub() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    auto diag = createDialog<PubSubDialog>(trViewChannelsTemplate_1S.arg(node->name()),
                                           GuiFactory::GetInstance().icon(server->GetType()), server, this);  // +
    VERIFY(connect(diag, &PubSubDialog::consoleOpenedAndExecute, this, &ExplorerTreeView::consoleOpenedAndExecute));
    diag->exec();
  }
}

void ExplorerTreeView::viewClientsMonitor() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerServerItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerServerItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    auto diag =
        createDialog<ClientsMonitorDialog>(trViewClientsTemplate_1S.arg(node->name()),
                                           GuiFactory::GetInstance().icon(server->GetType()), server, this);  // +
    diag->exec();
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
        QFileDialog::getOpenFileName(this, translations::trRestore, QString(), translations::trfilterForRdb);
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

    const QIcon dialog_icon = gui::GuiFactory::GetInstance().icon(node->server()->GetType());
    auto loadDb =
        createDialog<LoadContentDbDialog>(trLoadContentTemplate_1S.arg(node->name()), dialog_icon, this);  // +
    int result = loadDb->exec();
    if (result == QDialog::Accepted) {
      node->loadContent(common::ConvertToString(loadDb->pattern()), loadDb->count());
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

void ExplorerTreeView::remBranch() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerNSItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(ind);
    if (!node) {
      continue;
    }

    node->removeBranch();
  }
}

void ExplorerTreeView::renameBranch() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerNSItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(ind);
    if (!node) {
      continue;
    }

    QString name = node->name();
    common::qt::gui::RegExpInputDialog reg_dialog(this);
    reg_dialog.setWindowTitle(translations::trRenameBranch);
    reg_dialog.setLabelText(trRenameBranchLabel);
    reg_dialog.setText(name);
    QRegExp regExp(".*");
    reg_dialog.setRegExp(regExp);
    int result = reg_dialog.exec();
    if (result != QDialog::Accepted) {
      continue;
    }

    QString new_ns_name = reg_dialog.text();
    if (new_ns_name.isEmpty()) {
      continue;
    }

    if (new_ns_name == name) {
      continue;
    }

    node->renameBranch(name, new_ns_name);
  }
}

void ExplorerTreeView::addKeyToBranch() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerNSItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(ind);
    if (!node) {
      DNOTREACHED();
      continue;
    }

    proxy::IServerSPtr server = node->server();
    const core::nkey_t full_name_str(node->generateKeyTemplate(GEN_CMD_STRING("test")));
    const core::NKey raw_key(full_name_str);
    const core::NValue val(common::Value::CreateEmptyStringValue());
    const core::NDbKValue nkey(raw_key, val);
    ExplorerDatabaseItem* node_db = node->db();
    const auto inf = server->GetCurrentServerInfo();
    auto loadDb =
        createDialog<DbKeyDialog>(trCreateKeyForDbTemplate_1S.arg(node_db->name()), GuiFactory::GetInstance().keyIcon(),
                                  server->GetSupportedValueTypes(inf->GetVersion()), nkey, false, this);  // +
    int result = loadDb->exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb->key();
      node->createKey(key);
    }
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

    int answer =
        QMessageBox::question(this, translations::trRemoveDatabase, trRemoveDatabaseTemplate_1S.arg(node->name()),
                              QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes) {
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
    core::NValue val(common::Value::CreateEmptyStringValue());
    core::NDbKValue dbv(core::NKey(), val);
    const auto inf = server->GetCurrentServerInfo();
    auto loadDb =
        createDialog<DbKeyDialog>(trCreateKeyForDbTemplate_1S.arg(node->name()), GuiFactory::GetInstance().keyIcon(),
                                  server->GetSupportedValueTypes(inf->GetVersion()), dbv, false, this);  // +
    int result = loadDb->exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb->key();
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
    const auto inf = server->GetCurrentServerInfo();
    auto loadDb =
        createDialog<DbKeyDialog>(trEditKey_1S.arg(node->name()), GuiFactory::GetInstance().keyIcon(),
                                  server->GetSupportedValueTypes(inf->GetVersion()), node->dbv(), true, this);  // +
    int result = loadDb->exec();
    if (result == QDialog::Accepted) {
      core::NDbKValue key = loadDb->key();
      node->editValue(key.GetValue());
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

    auto diag = createDialog<ViewKeysDialog>(trViewKeyTemplate_1S.arg(node->name()), node->db(), this);  // +
    diag->exec();
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

void ExplorerTreeView::loadType() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (!node) {
      continue;
    }

    node->loadTypeFromDb();
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
    reg_dialog.setWindowTitle(translations::trRenameKey);
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

void ExplorerTreeView::remKey() {
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

void ExplorerTreeView::deleteItem() {
  QModelIndexList selected = selectedEqualTypeIndexes();
  for (QModelIndex ind : selected) {
    ExplorerKeyItem* node_key = common::qt::item<common::qt::gui::TreeItem*, ExplorerKeyItem*>(ind);
    if (node_key) {
      node_key->removeFromDb();
      continue;
    }

    ExplorerNSItem* node_ns = common::qt::item<common::qt::gui::TreeItem*, ExplorerNSItem*>(ind);
    if (node_ns) {
      node_ns->removeBranch();
      continue;
    }
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

void ExplorerTreeView::finishLoadDatabases(const proxy::events_info::LoadDatabasesInfoResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  proxy::events_info::LoadDatabasesInfoResponse::database_info_cont_type dbs = res.databases;
  for (size_t i = 0; i < dbs.size(); ++i) {
    core::IDataBaseInfoSPtr db = dbs[i];
    source_model_->addDatabase(serv, db);
  }
}

void ExplorerTreeView::startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  CHECK(serv);

  proxy::events_info::LoadDatabaseContentResponse::keys_container_t keys = res.keys;
  const std::string ns = serv->GetNsSeparator();
  proxy::NsDisplayStrategy ns_strategy = serv->GetNsDisplayStrategy();
  for (size_t i = 0; i < keys.size(); ++i) {
    core::NDbKValue key = keys[i];
    source_model_->addKey(serv, res.inf, key, ns, ns_strategy);
  }

  source_model_->updateDb(serv, res.inf);
}

void ExplorerTreeView::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ExplorerTreeView::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponse& res) {
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
  if (!serv) {
    return;
  }

  const std::string ns = serv->GetNsSeparator();
  const proxy::NsDisplayStrategy ns_strategy = serv->GetNsDisplayStrategy();
  source_model_->addKey(serv, db, key, ns, ns_strategy);
}

void ExplorerTreeView::renameKey(core::IDataBaseInfoSPtr db, core::NKey key, core::nkey_t new_name) {
  proxy::IServer* serv = qobject_cast<proxy::IServer*>(sender());
  if (!serv) {
    return;
  }

  core::NKey new_key = key;
  new_key.SetKey(new_name);
  const std::string ns = serv->GetNsSeparator();
  const proxy::NsDisplayStrategy ns_strategy = serv->GetNsDisplayStrategy();
  source_model_->renameKey(serv, db, key, new_key, ns, ns_strategy);
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
  loadValue();
  QTreeView::mouseDoubleClickEvent(e);
}

void ExplorerTreeView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Delete) {
    deleteItem();
  }
  return base_class::keyPressEvent(event);
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
