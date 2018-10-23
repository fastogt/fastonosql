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
#include <QEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "proxy/connection_settings/iconnection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "proxy/settings_manager.h"                          // for SettingsManager

#include "gui/dialogs/cluster_dialog.h"     // for ClusterDialog
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/connection_listwidget_items.h"
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
    : QDialog(parent), list_widget_(nullptr), ok_button_(nullptr) {
  setWindowTitle(title);
  setWindowIcon(icon);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  list_widget_ = new QTreeWidget;
  list_widget_->setIndentation(5);

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

  // list_widget_->setDragEnabled(true);
  // list_widget_->setDragDropMode(QAbstractItemView::InternalMove);
  setMinimumSize(QSize(min_width, min_height));
  VERIFY(connect(list_widget_, &QTreeWidget::itemDoubleClicked, this, &ConnectionsDialog::accept));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->button(QDialogButtonBox::Ok)->setIcon(GuiFactory::GetInstance().serverIcon());
  ok_button_ = buttonBox->button(QDialogButtonBox::Ok);

  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionsDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectionsDialog::reject));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(buttonBox);

  QToolBar* savebar = new QToolBar;

  QAction* addB = new QAction(GuiFactory::GetInstance().addIcon(), translations::trAddConnection, savebar);
  VERIFY(connect(addB, &QAction::triggered, this, &ConnectionsDialog::add));
  savebar->addAction(addB);

  QAction* addc = new QAction(GuiFactory::GetInstance().clusterIcon(), translations::trAddClusterConnection, savebar);
  VERIFY(connect(addc, &QAction::triggered, this, &ConnectionsDialog::addCls));
  savebar->addAction(addc);

  QAction* adds = new QAction(GuiFactory::GetInstance().sentinelIcon(), translations::trAddSentinelConnection, savebar);
  VERIFY(connect(adds, &QAction::triggered, this, &ConnectionsDialog::addSent));
  savebar->addAction(adds);

  QAction* editB = new QAction(GuiFactory::GetInstance().editIcon(), translations::trEditConnection, savebar);
  VERIFY(connect(editB, &QAction::triggered, this, &ConnectionsDialog::edit));
  savebar->addAction(editB);

  QAction* clone = new QAction(GuiFactory::GetInstance().cloneIcon(), translations::trCloneConnection, savebar);
  VERIFY(connect(clone, &QAction::triggered, this, &ConnectionsDialog::clone));
  savebar->addAction(clone);

  QAction* rmB = new QAction(GuiFactory::GetInstance().removeIcon(), translations::trRemoveConnection, savebar);
  VERIFY(connect(rmB, &QAction::triggered, this, &ConnectionsDialog::remove));
  savebar->addAction(rmB);

  QVBoxLayout* firstColumnLayout = new QVBoxLayout;
  firstColumnLayout->addWidget(savebar);
  firstColumnLayout->addWidget(list_widget_);
  firstColumnLayout->addLayout(bottomLayout);

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addLayout(firstColumnLayout, 1);

  // Populate list with connections
  auto connections = proxy::SettingsManager::GetInstance()->GetConnections();
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    proxy::IConnectionSettingsBaseSPtr connectionModel = (*it);
    addConnection(connectionModel);
  }

#if defined(PRO_VERSION)
  auto sentinels = proxy::SettingsManager::GetInstance()->GetSentinels();
  for (auto it = sentinels.begin(); it != sentinels.end(); ++it) {
    proxy::ISentinelSettingsBaseSPtr connectionModel = (*it);
    addSentinel(connectionModel);
  }

  auto clusters = proxy::SettingsManager::GetInstance()->GetClusters();
  for (auto it = clusters.begin(); it != clusters.end(); ++it) {
    proxy::IClusterSettingsBaseSPtr connectionModel = (*it);
    addCluster(connectionModel);
  }
#endif

  VERIFY(connect(list_widget_, &QTreeWidget::itemSelectionChanged, this, &ConnectionsDialog::itemSelectionChange));
  // Highlight first item
  if (list_widget_->topLevelItemCount() > 0) {
    list_widget_->setCurrentItem(list_widget_->topLevelItem(0));
  }
  retranslateUi();
}

proxy::IConnectionSettingsBaseSPtr ConnectionsDialog::selectedConnection() const {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return proxy::IConnectionSettingsBaseSPtr();
}

#if defined(PRO_VERSION)
proxy::ISentinelSettingsBaseSPtr ConnectionsDialog::selectedSentinel() const {
  SentinelConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<SentinelConnectionListWidgetItemContainer*>(list_widget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return proxy::ISentinelSettingsBaseSPtr();
}

proxy::IClusterSettingsBaseSPtr ConnectionsDialog::selectedCluster() const {
  ClusterConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<ClusterConnectionListWidgetItemContainer*>(list_widget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return proxy::IClusterSettingsBaseSPtr();
}
#endif

void ConnectionsDialog::add() {
  ConnectionSelectTypeDialog sel(trSelectConTypeTitle, this);
  int result = sel.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  core::ConnectionType t = sel.connectionType();
  ConnectionDialog dlg(t, translations::trNewConnection, this);
  result = dlg.exec();
  proxy::IConnectionSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    proxy::SettingsManager::GetInstance()->AddConnection(p);
    addConnection(p);
  }
}

void ConnectionsDialog::addCls() {
#if defined(PRO_VERSION)
  ClusterDialog dlg(this);
  int result = dlg.exec();
  proxy::IClusterSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    proxy::SettingsManager::GetInstance()->AddCluster(p);
    addCluster(p);
  }
#else
  QMessageBox::information(this, translations::trProLimitations, translations::trClustersAvailibleOnlyInProVersion);
#endif
}

void ConnectionsDialog::addSent() {
#if defined(PRO_VERSION)
  SentinelDialog dlg(this);
  int result = dlg.exec();
  proxy::ISentinelSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
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

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);
  ok_button_->setEnabled(!currentItem);
}

void ConnectionsDialog::edit() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  editItem(qitem, true);
}

void ConnectionsDialog::clone() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  editItem(qitem, false);
}

void ConnectionsDialog::remove() {
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

void ConnectionsDialog::editItem(QTreeWidgetItem* qitem, bool remove_origin) {
  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
#if defined(PRO_VERSION)
    IConnectionListWidgetItem::itemConnectionType type = currentItem->type();
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
        editConnection(currentItem, remove_origin);
        return;
      }
    } else if (type == IConnectionListWidgetItem::Sentinel) {
      qitem = qitem->parent();
    } else {
      NOTREACHED();
    }
#else
    editConnection(currentItem, remove_origin);
    return;
#endif
  }

#if defined(PRO_VERSION)
  if (ClusterConnectionListWidgetItemContainer* clCurrentItem =
          dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qitem)) {
    editCluster(clCurrentItem, remove_origin);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentCurrentItem =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    editSentinel(sentCurrentItem, remove_origin);
    return;
  }
#endif
}

void ConnectionsDialog::editConnection(ConnectionListWidgetItem* connectionItem, bool remove_origin) {
  CHECK(connectionItem);

  proxy::IConnectionSettingsBaseSPtr con = connectionItem->connection();
  ConnectionDialog dlg(con->Clone(), this);
  int result = dlg.exec();
  proxy::IConnectionSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    proxy::SettingsManager::GetInstance()->RemoveConnection(con);
    proxy::SettingsManager::GetInstance()->AddConnection(newConnection);

    if (remove_origin) {
      delete connectionItem;
    }
    addConnection(newConnection);
  }
}

#if defined(PRO_VERSION)
void ConnectionsDialog::editCluster(ClusterConnectionListWidgetItemContainer* clusterItem, bool remove_origin) {
  CHECK(clusterItem);

  proxy::IClusterSettingsBaseSPtr con = clusterItem->connection();
  ClusterDialog dlg(this, con->Clone());
  int result = dlg.exec();
  proxy::IClusterSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    proxy::SettingsManager::GetInstance()->RemoveCluster(con);
    proxy::SettingsManager::GetInstance()->AddCluster(newConnection);

    if (remove_origin) {
      delete clusterItem;
    }
    addCluster(newConnection);
  }
}

void ConnectionsDialog::editSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem, bool remove_origin) {
  CHECK(sentinelItem);

  proxy::ISentinelSettingsBaseSPtr con = sentinelItem->connection();
  SentinelDialog dlg(this, con->Clone());
  int result = dlg.exec();
  proxy::ISentinelSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    proxy::SettingsManager::GetInstance()->RemoveSentinel(con);
    proxy::SettingsManager::GetInstance()->AddSentinel(newConnection);

    if (remove_origin) {
      delete sentinelItem;
    }
    addSentinel(newConnection);
  }
}
#endif

void ConnectionsDialog::removeConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trDeleteConnectionTemplate_1S.arg(connectionItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  proxy::IConnectionSettingsBaseSPtr connection = connectionItem->connection();
  delete connectionItem;
  proxy::SettingsManager::GetInstance()->RemoveConnection(connection);
}

#if defined(PRO_VERSION)
void ConnectionsDialog::removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trDeleteClusterTemplate_1S.arg(clusterItem->text(0)),
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
                                     translations::trDeleteSentinelTemplate_1S.arg(sentinelItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  proxy::ISentinelSettingsBaseSPtr connection = sentinelItem->connection();
  delete sentinelItem;
  proxy::SettingsManager::GetInstance()->RemoveSentinel(connection);
}
#endif

void ConnectionsDialog::accept() {
  QTreeWidgetItem* qitem = list_widget_->currentItem();
  if (!qitem) {
    return;
  }

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);
  if (currentItem) {
    return;
  }

  QDialog::accept();
}

void ConnectionsDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ConnectionsDialog::retranslateUi() {
  ok_button_->setText(translations::trOpen);
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

#if defined(PRO_VERSION)
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
