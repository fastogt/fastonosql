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

#include "gui/dialogs/connections_dialog.h"

#include <memory>  // for __shared_ptr
#include <vector>  // for vector

#include <QAction>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "common/macros.h"  // for VERIFY, CHECK, NOTREACHED

#include "core/connection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "core/settings_manager.h"     // for SettingsManager

#include "gui/dialogs/cluster_dialog.h"     // for ClusterDialog
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/connection_listwidget_items.h"
#include "gui/dialogs/sentinel_dialog.h"  // for SentinelDialog
#include "gui/gui_factory.h"              // for GuiFactory

#include "translations/global.h"  // for trConnections, etc

namespace fastonosql {
namespace gui {

ConnectionsDialog::ConnectionsDialog(QWidget* parent) : QDialog(parent) {
  setWindowIcon(GuiFactory::instance().connectIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  listWidget_ = new QTreeWidget;
  listWidget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  listWidget_->setHeaderLabels(colums);

  // listWidget_->header()->setSectionResizeMode(0,
  // QHeaderView::Stretch);
  // listWidget_->header()->setSectionResizeMode(1,
  // QHeaderView::Stretch);

  // listWidget_->setViewMode(QListView::ListMode);
  listWidget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  listWidget_->setIndentation(15);
  listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item
                                                                      // can be draged
                                                                      // or
                                                                      // droped
  listWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  // listWidget_->setDragEnabled(true);
  // listWidget_->setDragDropMode(QAbstractItemView::InternalMove);
  setMinimumSize(QSize(min_width, min_height));
  VERIFY(connect(listWidget_, &QTreeWidget::itemDoubleClicked, this, &ConnectionsDialog::accept));

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->button(QDialogButtonBox::Ok)->setIcon(GuiFactory::instance().serverIcon());
  acButton_ = buttonBox->button(QDialogButtonBox::Ok);

  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionsDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectionsDialog::reject));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(buttonBox);

  QToolBar* savebar = new QToolBar;

  QAction* addB =
      new QAction(GuiFactory::instance().loadIcon(), translations::trAddConnection, savebar);
  VERIFY(connect(addB, &QAction::triggered, this, &ConnectionsDialog::add));
  savebar->addAction(addB);

  QAction* addc = new QAction(GuiFactory::instance().clusterIcon(),
                              translations::trAddClusterConnection, savebar);
  VERIFY(connect(addc, &QAction::triggered, this, &ConnectionsDialog::addCls));
  savebar->addAction(addc);

  QAction* adds = new QAction(GuiFactory::instance().sentinelIcon(),
                              translations::trAddSentinelConnection, savebar);
  VERIFY(connect(adds, &QAction::triggered, this, &ConnectionsDialog::addSent));
  savebar->addAction(adds);

  QAction* rmB =
      new QAction(GuiFactory::instance().removeIcon(), translations::trRemoveConnection, savebar);
  VERIFY(connect(rmB, &QAction::triggered, this, &ConnectionsDialog::remove));
  savebar->addAction(rmB);

  QAction* editB =
      new QAction(GuiFactory::instance().editIcon(), translations::trEditConnection, savebar);
  VERIFY(connect(editB, &QAction::triggered, this, &ConnectionsDialog::edit));
  savebar->addAction(editB);

  QVBoxLayout* firstColumnLayout = new QVBoxLayout;
  firstColumnLayout->addWidget(savebar);
  firstColumnLayout->addWidget(listWidget_);
  firstColumnLayout->addLayout(bottomLayout);

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addLayout(firstColumnLayout, 1);

  // Populate list with connections
  auto connections = core::SettingsManager::instance().connections();
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    core::IConnectionSettingsBaseSPtr connectionModel = (*it);
    addConnection(connectionModel);
  }

  auto sentinels = core::SettingsManager::instance().sentinels();
  for (auto it = sentinels.begin(); it != sentinels.end(); ++it) {
    core::ISentinelSettingsBaseSPtr connectionModel = (*it);
    addSentinel(connectionModel);
  }

  auto clusters = core::SettingsManager::instance().clusters();
  for (auto it = clusters.begin(); it != clusters.end(); ++it) {
    core::IClusterSettingsBaseSPtr connectionModel = (*it);
    addCluster(connectionModel);
  }

  VERIFY(connect(listWidget_, &QTreeWidget::itemSelectionChanged, this,
                 &ConnectionsDialog::itemSelectionChange));
  // Highlight first item
  if (listWidget_->topLevelItemCount() > 0) {
    listWidget_->setCurrentItem(listWidget_->topLevelItem(0));
  }
  retranslateUi();
}

core::IConnectionSettingsBaseSPtr ConnectionsDialog::selectedConnection() const {
  ConnectionListWidgetItem* currentItem =
      dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return core::IConnectionSettingsBaseSPtr();
}

core::ISentinelSettingsBaseSPtr ConnectionsDialog::selectedSentinel() const {
  SentinelConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<SentinelConnectionListWidgetItemContainer*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return core::ISentinelSettingsBaseSPtr();
}

core::IClusterSettingsBaseSPtr ConnectionsDialog::selectedCluster() const {
  ClusterConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<ClusterConnectionListWidgetItemContainer*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return core::IClusterSettingsBaseSPtr();
}

void ConnectionsDialog::add() {
  ConnectionDialog dlg(this);
  int result = dlg.exec();
  core::IConnectionSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().addConnection(p);
    addConnection(p);
  }
}

void ConnectionsDialog::addCls() {
  ClusterDialog dlg(this);
  int result = dlg.exec();
  core::IClusterSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().addCluster(p);
    addCluster(p);
  }
}

void ConnectionsDialog::addSent() {
  SentinelDialog dlg(this);
  int result = dlg.exec();
  core::ISentinelSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().addSentinel(p);
    addSentinel(p);
  }
}

void ConnectionsDialog::itemSelectionChange() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);
  acButton_->setEnabled(!currentItem);
}

void ConnectionsDialog::edit() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
    IConnectionListWidgetItem::itemConnectionType type = currentItem->type();
    if (type == IConnectionListWidgetItem::Common ||
        type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        editCluster(clitem);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        editSentinel(slitem);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem =
                     dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
        qitem = sslitem->parent();
      } else {
        editConnection(currentItem);
        return;
      }
    } else if (type == IConnectionListWidgetItem::Sentinel) {
      qitem = qitem->parent();
    } else {
      NOTREACHED();
    }
  }

  if (ClusterConnectionListWidgetItemContainer* clCurrentItem =
          dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qitem)) {
    editCluster(clCurrentItem);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentCurrentItem =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    editSentinel(sentCurrentItem);
    return;
  }
}

void ConnectionsDialog::remove() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
    IConnectionListWidgetItem::itemConnectionType type = currentItem->type();
    if (type == IConnectionListWidgetItem::Common ||
        type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        removeCluster(clitem);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        removeSentinel(slitem);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem =
                     dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
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
  }

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
}

void ConnectionsDialog::editConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  core::IConnectionSettingsBaseSPtr con = connectionItem->connection();
  ConnectionDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::IConnectionSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().removeConnection(con);
    core::SettingsManager::instance().addConnection(newConnection);

    delete connectionItem;
    addConnection(newConnection);
  }
}

void ConnectionsDialog::editCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  core::IClusterSettingsBaseSPtr con = clusterItem->connection();
  ClusterDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::IClusterSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().removeCluster(con);
    core::SettingsManager::instance().addCluster(newConnection);

    delete clusterItem;
    addCluster(newConnection);
  }
}

void ConnectionsDialog::editSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem) {
  CHECK(sentinelItem);

  core::ISentinelSettingsBaseSPtr con = sentinelItem->connection();
  SentinelDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::ISentinelSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().removeSentinel(con);
    core::SettingsManager::instance().addSentinel(newConnection);

    delete sentinelItem;
    addSentinel(newConnection);
  }
}

void ConnectionsDialog::removeConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  // Ask user
  int answer = QMessageBox::question(
      this, translations::trConnections,
      translations::trDeleteConnectionTemplate_1S.arg(connectionItem->text(0)), QMessageBox::Yes,
      QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::IConnectionSettingsBaseSPtr connection = connectionItem->connection();
  delete connectionItem;
  core::SettingsManager::instance().removeConnection(connection);
}

void ConnectionsDialog::removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  // Ask user
  int answer =
      QMessageBox::question(this, translations::trConnections,
                            translations::trDeleteClusterTemplate_1S.arg(clusterItem->text(0)),
                            QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::IClusterSettingsBaseSPtr connection = clusterItem->connection();
  delete clusterItem;
  core::SettingsManager::instance().removeCluster(connection);
}

void ConnectionsDialog::removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem) {
  CHECK(sentinelItem);

  // Ask user
  int answer =
      QMessageBox::question(this, translations::trConnections,
                            translations::trDeleteSentinelTemplate_1S.arg(sentinelItem->text(0)),
                            QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::ISentinelSettingsBaseSPtr connection = sentinelItem->connection();
  delete sentinelItem;
  core::SettingsManager::instance().removeSentinel(connection);
}

/**
 * @brief This function is called when user clicks on "Open"
 * button.
 */
void ConnectionsDialog::accept() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
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
  setWindowTitle(translations::trConnections);
  acButton_->setText(translations::trOpen);
}

void ConnectionsDialog::addConnection(core::IConnectionSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->path();
  core::IConnectionSettingsBase::connection_path_t dir(path.directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::root()) {
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(nullptr);
    item->setConnection(con);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(dirItem);
    item->setConnection(con);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::addCluster(core::IClusterSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->path();
  core::IConnectionSettingsBase::connection_path_t dir(path.directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::root()) {
    ClusterConnectionListWidgetItemContainer* item =
        new ClusterConnectionListWidgetItemContainer(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ClusterConnectionListWidgetItemContainer* item =
        new ClusterConnectionListWidgetItemContainer(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::addSentinel(core::ISentinelSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->path();
  core::IConnectionSettingsBase::connection_path_t dir(path.directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::root()) {
    SentinelConnectionListWidgetItemContainer* item =
        new SentinelConnectionListWidgetItemContainer(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    SentinelConnectionListWidgetItemContainer* item =
        new SentinelConnectionListWidgetItemContainer(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

DirectoryListWidgetItem* ConnectionsDialog::findFolderByPath(
    const core::IConnectionSettingsBase::connection_path_t& path) const {
  int count = listWidget_->topLevelItemCount();
  for (int i = 0; i < count; ++i) {
    QTreeWidgetItem* item = listWidget_->topLevelItem(i);
    DirectoryListWidgetItem* dirItem = dynamic_cast<DirectoryListWidgetItem*>(item);  // +
    if (dirItem && dirItem->path() == path) {
      return dirItem;
    }
  }
  return nullptr;
}

}  // namespace gui
}  // namespace fastonosql
