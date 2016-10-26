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

#include <common/macros.h>  // for VERIFY, CHECK, NOTREACHED

#include "core/connection_settings/connection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "core/settings_manager.h"                         // for SettingsManager

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
  VERIFY(connect(addB, &QAction::triggered, this, &ConnectionsDialog::Add));
  savebar->addAction(addB);

  QAction* addc = new QAction(GuiFactory::instance().clusterIcon(),
                              translations::trAddClusterConnection, savebar);
  VERIFY(connect(addc, &QAction::triggered, this, &ConnectionsDialog::AddCls));
  savebar->addAction(addc);

  QAction* adds = new QAction(GuiFactory::instance().sentinelIcon(),
                              translations::trAddSentinelConnection, savebar);
  VERIFY(connect(adds, &QAction::triggered, this, &ConnectionsDialog::AddSent));
  savebar->addAction(adds);

  QAction* rmB =
      new QAction(GuiFactory::instance().removeIcon(), translations::trRemoveConnection, savebar);
  VERIFY(connect(rmB, &QAction::triggered, this, &ConnectionsDialog::Remove));
  savebar->addAction(rmB);

  QAction* editB =
      new QAction(GuiFactory::instance().editIcon(), translations::trEditConnection, savebar);
  VERIFY(connect(editB, &QAction::triggered, this, &ConnectionsDialog::Edit));
  savebar->addAction(editB);

  QVBoxLayout* firstColumnLayout = new QVBoxLayout;
  firstColumnLayout->addWidget(savebar);
  firstColumnLayout->addWidget(listWidget_);
  firstColumnLayout->addLayout(bottomLayout);

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addLayout(firstColumnLayout, 1);

  // Populate list with connections
  auto connections = core::SettingsManager::instance().Connections();
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    core::IConnectionSettingsBaseSPtr connectionModel = (*it);
    AddConnection(connectionModel);
  }

  auto sentinels = core::SettingsManager::instance().Sentinels();
  for (auto it = sentinels.begin(); it != sentinels.end(); ++it) {
    core::ISentinelSettingsBaseSPtr connectionModel = (*it);
    AddSentinel(connectionModel);
  }

  auto clusters = core::SettingsManager::instance().Clusters();
  for (auto it = clusters.begin(); it != clusters.end(); ++it) {
    core::IClusterSettingsBaseSPtr connectionModel = (*it);
    AddCluster(connectionModel);
  }

  VERIFY(connect(listWidget_, &QTreeWidget::itemSelectionChanged, this,
                 &ConnectionsDialog::ItemSelectionChange));
  // Highlight first item
  if (listWidget_->topLevelItemCount() > 0) {
    listWidget_->setCurrentItem(listWidget_->topLevelItem(0));
  }
  RetranslateUi();
}

core::IConnectionSettingsBaseSPtr ConnectionsDialog::SelectedConnection() const {
  ConnectionListWidgetItem* currentItem =
      dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->Connection();
  }

  return core::IConnectionSettingsBaseSPtr();
}

core::ISentinelSettingsBaseSPtr ConnectionsDialog::SelectedSentinel() const {
  SentinelConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<SentinelConnectionListWidgetItemContainer*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->Connection();
  }

  return core::ISentinelSettingsBaseSPtr();
}

core::IClusterSettingsBaseSPtr ConnectionsDialog::SelectedCluster() const {
  ClusterConnectionListWidgetItemContainer* currentItem =
      dynamic_cast<ClusterConnectionListWidgetItemContainer*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->Connection();
  }

  return core::IClusterSettingsBaseSPtr();
}

void ConnectionsDialog::Add() {
  ConnectionDialog dlg(this);
  int result = dlg.exec();
  core::IConnectionSettingsBaseSPtr p = dlg.Connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().AddConnection(p);
    AddConnection(p);
  }
}

void ConnectionsDialog::AddCls() {
  ClusterDialog dlg(this);
  int result = dlg.exec();
  core::IClusterSettingsBaseSPtr p = dlg.Connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().AddCluster(p);
    AddCluster(p);
  }
}

void ConnectionsDialog::AddSent() {
  SentinelDialog dlg(this);
  int result = dlg.exec();
  core::ISentinelSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    core::SettingsManager::instance().AddSentinel(p);
    AddSentinel(p);
  }
}

void ConnectionsDialog::ItemSelectionChange() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  DirectoryListWidgetItem* currentItem = dynamic_cast<DirectoryListWidgetItem*>(qitem);
  acButton_->setEnabled(!currentItem);
}

void ConnectionsDialog::Edit() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
    IConnectionListWidgetItem::itemConnectionType type = currentItem->Type();
    if (type == IConnectionListWidgetItem::Common ||
        type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        EditCluster(clitem);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        EditSentinel(slitem);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem =
                     dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
        qitem = sslitem->parent();
      } else {
        EditConnection(currentItem);
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
    EditCluster(clCurrentItem);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentCurrentItem =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    EditSentinel(sentCurrentItem);
    return;
  }
}

void ConnectionsDialog::Remove() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  if (ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem)) {
    IConnectionListWidgetItem::itemConnectionType type = currentItem->Type();
    if (type == IConnectionListWidgetItem::Common ||
        type == IConnectionListWidgetItem::Discovered) {
      QTreeWidgetItem* qpitem = qitem->parent();
      if (ClusterConnectionListWidgetItemContainer* clitem =
              dynamic_cast<ClusterConnectionListWidgetItemContainer*>(qpitem)) {
        RemoveCluster(clitem);
        return;
      } else if (SentinelConnectionListWidgetItemContainer* slitem =
                     dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
        RemoveSentinel(slitem);
        return;
      } else if (SentinelConnectionWidgetItem* sslitem =
                     dynamic_cast<SentinelConnectionWidgetItem*>(qpitem)) {
        qitem = sslitem->parent();
      } else {
        RemoveConnection(currentItem);
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
    RemoveCluster(clCurrentItem);
    return;
  }

  if (SentinelConnectionListWidgetItemContainer* sentCurrentItem =
          dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem)) {
    RemoveSentinel(sentCurrentItem);
    return;
  }
}

void ConnectionsDialog::EditConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  core::IConnectionSettingsBaseSPtr con = connectionItem->Connection();
  ConnectionDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::IConnectionSettingsBaseSPtr newConnection = dlg.Connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().RemoveConnection(con);
    core::SettingsManager::instance().AddConnection(newConnection);

    delete connectionItem;
    AddConnection(newConnection);
  }
}

void ConnectionsDialog::EditCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  core::IClusterSettingsBaseSPtr con = clusterItem->Connection();
  ClusterDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::IClusterSettingsBaseSPtr newConnection = dlg.Connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().RemoveCluster(con);
    core::SettingsManager::instance().AddCluster(newConnection);

    delete clusterItem;
    AddCluster(newConnection);
  }
}

void ConnectionsDialog::EditSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem) {
  CHECK(sentinelItem);

  core::ISentinelSettingsBaseSPtr con = sentinelItem->Connection();
  SentinelDialog dlg(this, con->Clone());
  int result = dlg.exec();
  core::ISentinelSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    core::SettingsManager::instance().RemoveSentinel(con);
    core::SettingsManager::instance().AddSentinel(newConnection);

    delete sentinelItem;
    AddSentinel(newConnection);
  }
}

void ConnectionsDialog::RemoveConnection(ConnectionListWidgetItem* connectionItem) {
  CHECK(connectionItem);

  // Ask user
  int answer = QMessageBox::question(
      this, translations::trConnections,
      translations::trDeleteConnectionTemplate_1S.arg(connectionItem->text(0)), QMessageBox::Yes,
      QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::IConnectionSettingsBaseSPtr connection = connectionItem->Connection();
  delete connectionItem;
  core::SettingsManager::instance().RemoveConnection(connection);
}

void ConnectionsDialog::RemoveCluster(ClusterConnectionListWidgetItemContainer* clusterItem) {
  CHECK(clusterItem);

  // Ask user
  int answer =
      QMessageBox::question(this, translations::trConnections,
                            translations::trDeleteClusterTemplate_1S.arg(clusterItem->text(0)),
                            QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::IClusterSettingsBaseSPtr connection = clusterItem->Connection();
  delete clusterItem;
  core::SettingsManager::instance().RemoveCluster(connection);
}

void ConnectionsDialog::RemoveSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem) {
  CHECK(sentinelItem);

  // Ask user
  int answer =
      QMessageBox::question(this, translations::trConnections,
                            translations::trDeleteSentinelTemplate_1S.arg(sentinelItem->text(0)),
                            QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  core::ISentinelSettingsBaseSPtr connection = sentinelItem->Connection();
  delete sentinelItem;
  core::SettingsManager::instance().RemoveSentinel(connection);
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
    RetranslateUi();
  }
  QDialog::changeEvent(e);
}

void ConnectionsDialog::RetranslateUi() {
  setWindowTitle(translations::trConnections);
  acButton_->setText(translations::trOpen);
}

void ConnectionsDialog::AddConnection(core::IConnectionSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->Path();
  core::IConnectionSettingsBase::connection_path_t dir(path.Directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::Root()) {
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(nullptr);
    item->SetConnection(con);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = FindFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(dirItem);
    item->SetConnection(con);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::AddCluster(core::IClusterSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->Path();
  core::IConnectionSettingsBase::connection_path_t dir(path.Directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::Root()) {
    ClusterConnectionListWidgetItemContainer* item =
        new ClusterConnectionListWidgetItemContainer(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = FindFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ClusterConnectionListWidgetItemContainer* item =
        new ClusterConnectionListWidgetItemContainer(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::AddSentinel(core::ISentinelSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->Path();
  core::IConnectionSettingsBase::connection_path_t dir(path.Directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::Root()) {
    SentinelConnectionListWidgetItemContainer* item =
        new SentinelConnectionListWidgetItemContainer(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = FindFolderByPath(dir);
    if (!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    SentinelConnectionListWidgetItemContainer* item =
        new SentinelConnectionListWidgetItemContainer(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

DirectoryListWidgetItem* ConnectionsDialog::FindFolderByPath(
    const core::IConnectionSettingsBase::connection_path_t& path) const {
  int count = listWidget_->topLevelItemCount();
  for (int i = 0; i < count; ++i) {
    QTreeWidgetItem* item = listWidget_->topLevelItem(i);
    DirectoryListWidgetItem* dirItem = dynamic_cast<DirectoryListWidgetItem*>(item);  // +
    if (dirItem && dirItem->Path() == path) {
      return dirItem;
    }
  }
  return nullptr;
}

}  // namespace gui
}  // namespace fastonosql
