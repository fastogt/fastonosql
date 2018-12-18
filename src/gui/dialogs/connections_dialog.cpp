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

#include "gui/dialogs/connections_dialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "proxy/connection_settings/iconnection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "proxy/settings_manager.h"                          // for SettingsManager

#include "gui/connection_listwidget_items.h"
#include "gui/dialogs/cluster_dialog.h"     // for ClusterDialog
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/connection_select_type_dialog.h"
#include "gui/dialogs/sentinel_dialog.h"  // for SentinelDialog
#include "gui/gui_factory.h"              // for GuiFactory

#include "translations/global.h"  // for trConnections, etc

namespace {
const QString trSelectConTypeTitle = QObject::tr("Select connection type");
}

namespace fastonosql {
namespace gui {

ConnectionsDialog::ConnectionsDialog(const QString& title, const QIcon& icon, QWidget* parent)
    : base_class(title, parent), list_widget_(nullptr), ok_button_(nullptr) {
  setWindowIcon(icon);

  list_widget_ = new QTreeWidget;

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  list_widget_->setHeaderLabels(colums);

  // list_widget_->header()->setSectionResizeMode(0,
  // QHeaderView::Stretch);
  // list_widget_->header()->setSectionResizeMode(1,
  // QHeaderView::Stretch);

  // list_widget_->setViewMode(QListView::ListMode);
  list_widget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  list_widget_->setIndentation(15);
  list_widget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item can be draged or droped
  list_widget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  list_widget_->header()->resizeSection(0, min_width / 3);

  // list_widget_->setDragEnabled(true);
  // list_widget_->setDragDropMode(QAbstractItemView::InternalMove);
  VERIFY(connect(list_widget_, &QTreeWidget::itemDoubleClicked, this, &ConnectionsDialog::accept));

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  button_box->button(QDialogButtonBox::Ok)->setIcon(GuiFactory::GetInstance().serverIcon());
  ok_button_ = button_box->button(QDialogButtonBox::Ok);

  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &ConnectionsDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &ConnectionsDialog::reject));

  // Populate list with connections
  auto connections = proxy::SettingsManager::GetInstance()->GetConnections();
  for (proxy::IConnectionSettingsBaseSPtr connection_model : connections) {
    addConnection(connection_model);
  }

#if defined(PRO_VERSION)
  auto sentinels = proxy::SettingsManager::GetInstance()->GetSentinels();
  for (proxy::ISentinelSettingsBaseSPtr connection_model : sentinels) {
    addSentinel(connection_model);
  }

  auto clusters = proxy::SettingsManager::GetInstance()->GetClusters();
  for (proxy::IClusterSettingsBaseSPtr connection_model : clusters) {
    addCluster(connection_model);
  }
#endif

  VERIFY(connect(list_widget_, &QTreeWidget::itemSelectionChanged, this, &ConnectionsDialog::itemSelectionChange));
  // Highlight first item
  if (list_widget_->topLevelItemCount() > 0) {
    list_widget_->setCurrentItem(list_widget_->topLevelItem(0));
  }

  QToolBar* savebar = createToolBar();

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(savebar);
  main_layout->addWidget(list_widget_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);

  setMinimumSize(QSize(min_width, min_height));
}

proxy::IConnectionSettingsBaseSPtr ConnectionsDialog::selectedConnection() const {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +
  if (current_item) {
    return current_item->connection();
  }

  return proxy::IConnectionSettingsBaseSPtr();
}

#if defined(PRO_VERSION)
proxy::ISentinelSettingsBaseSPtr ConnectionsDialog::selectedSentinel() const {
  SentinelConnectionListWidgetItemContainer* current_item =
      dynamic_cast<SentinelConnectionListWidgetItemContainer*>(list_widget_->currentItem());  // +
  if (current_item) {
    return current_item->connection();
  }

  return proxy::ISentinelSettingsBaseSPtr();
}

proxy::IClusterSettingsBaseSPtr ConnectionsDialog::selectedCluster() const {
  ClusterConnectionListWidgetItemContainer* current_item =
      dynamic_cast<ClusterConnectionListWidgetItemContainer*>(list_widget_->currentItem());  // +
  if (current_item) {
    return current_item->connection();
  }

  return proxy::IClusterSettingsBaseSPtr();
}
#endif

void ConnectionsDialog::addConnectionAction() {
  auto sel = createDialog<ConnectionSelectTypeDialog>(trSelectConTypeTitle, this);  // +
  int result = sel->exec();
  if (result != QDialog::Accepted) {
    return;
  }

  core::ConnectionType t = sel->connectionType();
  auto dlg = createDialog<ConnectionDialog>(t, translations::trNewConnection, this);  // +
  result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IConnectionSettingsBaseSPtr p = dlg->connection();
    proxy::SettingsManager::GetInstance()->AddConnection(p);
    addConnection(p);
  }
}

void ConnectionsDialog::addClusterAction() {
#if defined(PRO_VERSION)
  auto dlg = createDialog<ClusterDialog>(nullptr, this);  // +
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IClusterSettingsBaseSPtr p = dlg->connection();
    proxy::SettingsManager::GetInstance()->AddCluster(p);
    addCluster(p);
  }
#else
  QMessageBox::information(this, translations::trProLimitations, translations::trClustersAvailibleOnlyInProVersion);
#endif
}

void ConnectionsDialog::addSentinelAction() {
#if defined(PRO_VERSION)
  auto dlg = createDialog<SentinelDialog>(nullptr, this);  // +
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::ISentinelSettingsBaseSPtr p = dlg->connection();
    proxy::SettingsManager::GetInstance()->AddSentinel(p);
    addSentinel(p);
  }
#else
  QMessageBox::information(this, translations::trProLimitations, translations::trSentinelsAvailibleOnlyInProVersion);
#endif
}

void ConnectionsDialog::itemSelectionChange() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);  // +
  ok_button_->setEnabled(!currentItem);
}

void ConnectionsDialog::editItemAction() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  editItem(qitem, true);
}

void ConnectionsDialog::cloneItemAction() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  editItem(qitem, false);
}

void ConnectionsDialog::removeItemAction() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
#if defined(PRO_VERSION)
    IConnectionListWidgetItem::itemConnectionType type = currentItem->type();
    if (type == IConnectionListWidgetItem::Common || type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        removeCluster(clitem);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        removeSentinel(slitem);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem = dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
        qitem = sslitem->parent();
      } else {
        removeConnection(currentItem);
        return;
      }
    } else if (type == IConnectionListWidgetItem::Sentinel) {
      qitem = qitem->parent();
    } else {
      NOTREACHED();
    }
#else
    removeConnection(currentItem);
    return;
#endif
  }

#if defined(PRO_VERSION)
  if (ClusterConnectionListWidgetItemContainer* clCurrentItem =
          dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qitem)) {
    removeCluster(clCurrentItem);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentCurrentItem =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    removeSentinel(sentCurrentItem);
    return;
  }
#endif
}

QToolBar* ConnectionsDialog::createToolBar() {
  QToolBar* savebar = new QToolBar;

  add_connection_action_ = new QAction;
  add_connection_action_->setIcon(GuiFactory::GetInstance().addIcon());
  VERIFY(connect(add_connection_action_, &QAction::triggered, this, &ConnectionsDialog::addConnectionAction));
  savebar->addAction(add_connection_action_);

  add_cluster_action_ = new QAction;
  add_cluster_action_->setIcon(GuiFactory::GetInstance().clusterIcon());
  VERIFY(connect(add_cluster_action_, &QAction::triggered, this, &ConnectionsDialog::addClusterAction));
  savebar->addAction(add_cluster_action_);

  add_sentinel_action_ = new QAction;
  add_sentinel_action_->setIcon(GuiFactory::GetInstance().sentinelIcon());
  VERIFY(connect(add_sentinel_action_, &QAction::triggered, this, &ConnectionsDialog::addSentinelAction));
  savebar->addAction(add_sentinel_action_);

  edit_action_ = new QAction;
  edit_action_->setIcon(GuiFactory::GetInstance().editIcon());
  VERIFY(connect(edit_action_, &QAction::triggered, this, &ConnectionsDialog::editItemAction));
  savebar->addAction(edit_action_);

  clone_action_ = new QAction;
  clone_action_->setIcon(GuiFactory::GetInstance().cloneIcon());
  VERIFY(connect(clone_action_, &QAction::triggered, this, &ConnectionsDialog::cloneItemAction));
  savebar->addAction(clone_action_);

  remove_action_ = new QAction;
  remove_action_->setIcon(GuiFactory::GetInstance().removeIcon());
  VERIFY(connect(remove_action_, &QAction::triggered, this, &ConnectionsDialog::removeItemAction));
  savebar->addAction(remove_action_);
  return savebar;
}

void ConnectionsDialog::editItem(QTreeWidgetItem* qitem, bool remove_origin) {
  if (ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
#if defined(PRO_VERSION)
    IConnectionListWidgetItem::itemConnectionType type = current_item->type();
    if (type == IConnectionListWidgetItem::Common || type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        editCluster(clitem, remove_origin);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        editSentinel(slitem, remove_origin);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem = dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
        qitem = sslitem->parent();
      } else {
        editConnection(current_item, remove_origin);
        return;
      }
    } else if (type == IConnectionListWidgetItem::Sentinel) {
      qitem = qitem->parent();
    } else {
      NOTREACHED();
    }
#else
    editConnection(current_item, remove_origin);
    return;
#endif
  }

#if defined(PRO_VERSION)
  if (ClusterConnectionListWidgetItemContainer* cluster_item =
          dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qitem)) {
    editCluster(cluster_item, remove_origin);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentinel_item =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    editSentinel(sentinel_item, remove_origin);
    return;
  }
#endif
}

void ConnectionsDialog::editConnection(ConnectionListWidgetItem* connection_item, bool remove_origin) {
  CHECK(connection_item);

  proxy::IConnectionSettingsBaseSPtr con = connection_item->connection();
  auto dlg = createDialog<ConnectionDialog>(con->Clone(), this);  // +
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IConnectionSettingsBaseSPtr connection = dlg->connection();
    proxy::SettingsManager::GetInstance()->RemoveConnection(con);
    proxy::SettingsManager::GetInstance()->AddConnection(connection);

    if (remove_origin) {
      delete connection_item;
    }
    addConnection(connection);
  }
}

#if defined(PRO_VERSION)
void ConnectionsDialog::editCluster(ClusterConnectionListWidgetItemContainer* cluster_item, bool remove_origin) {
  CHECK(cluster_item);

  proxy::IClusterSettingsBaseSPtr con = cluster_item->connection();
  auto dlg = createDialog<ClusterDialog>(con->Clone(), this);  // +
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IClusterSettingsBaseSPtr new_connection = dlg->connection();
    proxy::SettingsManager::GetInstance()->RemoveCluster(con);
    proxy::SettingsManager::GetInstance()->AddCluster(new_connection);

    if (remove_origin) {
      delete cluster_item;
    }
    addCluster(new_connection);
  }
}

void ConnectionsDialog::editSentinel(SentinelConnectionListWidgetItemContainer* sentinel_item, bool remove_origin) {
  CHECK(sentinel_item);

  proxy::ISentinelSettingsBaseSPtr con = sentinel_item->connection();
  auto dlg = createDialog<SentinelDialog>(con->Clone(), this);  // +
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::ISentinelSettingsBaseSPtr new_connection = dlg->connection();
    proxy::SettingsManager::GetInstance()->RemoveSentinel(con);
    proxy::SettingsManager::GetInstance()->AddSentinel(new_connection);

    if (remove_origin) {
      delete sentinel_item;
    }
    addSentinel(new_connection);
  }
}

void ConnectionsDialog::removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trRemoveClusterTemplate_1S.arg(clusterItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  proxy::IClusterSettingsBaseSPtr connection = clusterItem->connection();
  delete clusterItem;
  proxy::SettingsManager::GetInstance()->RemoveCluster(connection);
}

void ConnectionsDialog::removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem) {
  CHECK(sentinelItem);

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trRemoveSentinelTemplate_1S.arg(sentinelItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  proxy::ISentinelSettingsBaseSPtr connection = sentinelItem->connection();
  delete sentinelItem;
  proxy::SettingsManager::GetInstance()->RemoveSentinel(connection);
}

void ConnectionsDialog::addCluster(proxy::IClusterSettingsBaseSPtr con) {
  proxy::connection_path_t path = con->GetPath();
  proxy::connection_path_t dir(path.GetDirectory());
  if (dir == proxy::connection_path_t::GetRoot()) {
    ClusterConnectionListWidgetItemContainer* item = new ClusterConnectionListWidgetItemContainer(con, nullptr);
    list_widget_->addTopLevelItem(item);
    return;
  }

  DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
  if (!dirItem) {
    dirItem = new DirectoryListWidgetItem(dir);
  }

  ClusterConnectionListWidgetItemContainer* item = new ClusterConnectionListWidgetItemContainer(con, dirItem);
  dirItem->addChild(item);
  list_widget_->addTopLevelItem(dirItem);
}

void ConnectionsDialog::addSentinel(proxy::ISentinelSettingsBaseSPtr con) {
  proxy::connection_path_t path = con->GetPath();
  proxy::connection_path_t dir(path.GetDirectory());
  if (dir == proxy::connection_path_t::GetRoot()) {
    SentinelConnectionListWidgetItemContainer* item = new SentinelConnectionListWidgetItemContainer(con, nullptr);
    list_widget_->addTopLevelItem(item);
    return;
  }

  DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
  if (!dirItem) {
    dirItem = new DirectoryListWidgetItem(dir);
  }

  SentinelConnectionListWidgetItemContainer* item = new SentinelConnectionListWidgetItemContainer(con, dirItem);
  dirItem->addChild(item);
  list_widget_->addTopLevelItem(dirItem);
}
#endif

void ConnectionsDialog::removeConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trRemoveConnectionTemplate_1S.arg(connectionItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  proxy::IConnectionSettingsBaseSPtr connection = connectionItem->connection();
  delete connectionItem;
  proxy::SettingsManager::GetInstance()->RemoveConnection(connection);
}

void ConnectionsDialog::accept() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);
  if (currentItem) {
    return;
  }

  base_class::accept();
}

void ConnectionsDialog::retranslateUi() {
  add_connection_action_->setToolTip(translations::trAddConnection);
  add_cluster_action_->setToolTip(translations::trAddClusterConnection);
  add_sentinel_action_->setToolTip(translations::trAddSentinelConnection);
  edit_action_->setToolTip(translations::trEditConnection);
  clone_action_->setToolTip(translations::trCloneConnection);
  remove_action_->setToolTip(translations::trRemoveConnection);

  ok_button_->setText(translations::trOpen);
  base_class::retranslateUi();
}

void ConnectionsDialog::addConnection(proxy::IConnectionSettingsBaseSPtr con) {
  proxy::connection_path_t path = con->GetPath();
  proxy::connection_path_t dir(path.GetDirectory());
  if (dir == proxy::connection_path_t::GetRoot()) {
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(nullptr);
    item->setConnection(con);
    list_widget_->addTopLevelItem(item);
    return;
  }

  DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
  if (!dirItem) {
    dirItem = new DirectoryListWidgetItem(dir);
  }

  ConnectionListWidgetItem* item = new ConnectionListWidgetItem(dirItem);
  item->setConnection(con);
  dirItem->addChild(item);
  list_widget_->addTopLevelItem(dirItem);
}

DirectoryListWidgetItem* ConnectionsDialog::findFolderByPath(const proxy::connection_path_t& path) const {
  int count = list_widget_->topLevelItemCount();
  for (int i = 0; i < count; ++i) {
    QTreeWidgetItem* item = list_widget_->topLevelItem(i);
    DirectoryListWidgetItem* dirItem = dynamic_cast<DirectoryListWidgetItem*>(item);  // +
    if (dirItem && dirItem->path() == path) {
      return dirItem;
    }
  }
  return nullptr;
}

}  // namespace gui
}  // namespace fastonosql
