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

#include <QHeaderView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QEvent>
#include <QToolBar>
#include <QAction>

#include "common/qt/convert_string.h"

#include "core/settings_manager.h"
#include "core/connection_settings.h"

#include "gui/gui_factory.h"
#include "gui/dialogs/connection_listwidget_items.h"
#include "gui/dialogs/connection_dialog.h"
#include "gui/dialogs/cluster_dialog.h"
#include "gui/dialogs/sentinel_dialog.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

ConnectionsDialog::ConnectionsDialog(QWidget* parent)
  : QDialog(parent) {
  setWindowIcon(GuiFactory::instance().connectIcon());

  // Remove help button (?)
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  listWidget_ = new QTreeWidget;
  listWidget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  listWidget_->setHeaderLabels(colums);

  // listWidget_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  // listWidget_->header()->setSectionResizeMode(1, QHeaderView::Stretch);

  // listWidget_->setViewMode(QListView::ListMode);
  listWidget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  listWidget_->setIndentation(15);
  listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item can be draged or droped
  listWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  // listWidget_->setDragEnabled(true);
  // listWidget_->setDragDropMode(QAbstractItemView::InternalMove);
  setMinimumSize(QSize(min_width, min_height));
  VERIFY(connect(listWidget_, &QTreeWidget::itemDoubleClicked, this, &ConnectionsDialog::accept));
  VERIFY(connect(listWidget_, &QTreeWidget::itemSelectionChanged,
                 this, &ConnectionsDialog::connectionSelectChange));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->button(QDialogButtonBox::Ok)->setIcon(GuiFactory::instance().serverIcon());
  acButton_ = buttonBox->button(QDialogButtonBox::Ok);
  acButton_->setEnabled(false);

  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionsDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectionsDialog::reject));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(buttonBox);

  QToolBar* savebar = new QToolBar;

  QAction* addB = new QAction(GuiFactory::instance().loadIcon(),
                              translations::trAddConnection, savebar);
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

  QAction* rmB = new QAction(GuiFactory::instance().removeIcon(),
                             translations::trRemoveConnection, savebar);
  VERIFY(connect(rmB, &QAction::triggered, this, &ConnectionsDialog::remove));
  savebar->addAction(rmB);

  QAction* editB = new QAction(GuiFactory::instance().editIcon(),
                               translations::trEditConnection, savebar);
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

  // Highlight first item
  if (listWidget_->topLevelItemCount() > 0) {
    listWidget_->setCurrentItem(listWidget_->topLevelItem(0));
  }
  retranslateUi();
}

core::IConnectionSettingsBaseSPtr ConnectionsDialog::selectedConnection() const {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return core::IConnectionSettingsBaseSPtr();
}

core::ISentinelSettingsBaseSPtr ConnectionsDialog::selectedSentinel() const {
  SentinelConnectionListWidgetItemContainer* currentItem = dynamic_cast<SentinelConnectionListWidgetItemContainer*>(listWidget_->currentItem());  // +
  if (currentItem) {
    return currentItem->connection();
  }

  return core::ISentinelSettingsBaseSPtr();
}

core::IClusterSettingsBaseSPtr ConnectionsDialog::selectedCluster() const {
  ClusterConnectionListWidgetItem* currentItem = dynamic_cast<ClusterConnectionListWidgetItem*>(listWidget_->currentItem());  // +
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

void ConnectionsDialog::remove() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem);  // +
  if (currentItem) {
    QTreeWidgetItem* qpitem = qitem->parent();
    if (ClusterConnectionListWidgetItem* clitem = dynamic_cast<ClusterConnectionListWidgetItem*>(qpitem)) {
      qitem = clitem;
    } else if (SentinelConnectionListWidgetItemContainer* slitem = dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
      qitem = slitem;
    } else {
      // Ask user
      int answer = QMessageBox::question(this, "Connections",
                                         QString("Really delete \"%1\" connection?").arg(currentItem->text(0)),
                                         QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (answer != QMessageBox::Yes)
        return;

      core::IConnectionSettingsBaseSPtr connection = currentItem->connection();
      delete currentItem;
      core::SettingsManager::instance().removeConnection(connection);
      return;
    }
  }

  ClusterConnectionListWidgetItem* clCurrentItem = dynamic_cast<ClusterConnectionListWidgetItem*>(qitem);  // +
  if (clCurrentItem) {
    // Ask user
    int answer = QMessageBox::question(this, "Connections",
                                       QString("Really delete \"%1\" cluster?").arg(clCurrentItem->text(0)),
                                       QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes)
      return;

    core::IClusterSettingsBaseSPtr connection = clCurrentItem->connection();
    delete clCurrentItem;
    core::SettingsManager::instance().removeCluster(connection);
  }

  SentinelConnectionListWidgetItemContainer* sentCurrentItem = dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem);  // +
  if (sentCurrentItem) {
    // Ask user
    int answer = QMessageBox::question(this, "Connections",
                                       QString("Really delete \"%1\" sentinel?").arg(clCurrentItem->text(0)),
                                       QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

    if (answer != QMessageBox::Yes)
      return;

    core::ISentinelSettingsBaseSPtr connection = sentCurrentItem->connection();
    delete sentCurrentItem;
    core::SettingsManager::instance().removeSentinel(connection);
  }
}

void ConnectionsDialog::edit() {
  QTreeWidgetItem* qitem = listWidget_->currentItem();
  if (!qitem) {
    return;
  }

  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(qitem);  // +
  if (currentItem) {
    QTreeWidgetItem* qpitem = qitem->parent();
    if (ClusterConnectionListWidgetItem* clitem = dynamic_cast<ClusterConnectionListWidgetItem*>(qpitem)) {
      qitem = clitem;
    } else if (SentinelConnectionListWidgetItemContainer* slitem = dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qpitem)) {
      qitem = slitem;
    } else {
      core::IConnectionSettingsBaseSPtr con = currentItem->connection();
      ConnectionDialog dlg(this, con->clone());
      int result = dlg.exec();
      core::IConnectionSettingsBaseSPtr newConnection = dlg.connection();
      if (result == QDialog::Accepted && newConnection) {
        currentItem->setConnection(newConnection);
        core::SettingsManager::instance().removeConnection(con);
        core::SettingsManager::instance().addConnection(newConnection);

        delete currentItem;
        addConnection(newConnection);
      }
      return;
    }
  }

  ClusterConnectionListWidgetItem* clCurrentItem = dynamic_cast<ClusterConnectionListWidgetItem*>(qitem);  // +
  if (clCurrentItem) {
    core::IClusterSettingsBaseSPtr con = clCurrentItem->connection();
    ClusterDialog dlg(this, con->clone());
    int result = dlg.exec();
    core::IClusterSettingsBaseSPtr newConnection = dlg.connection();
    if (result == QDialog::Accepted && newConnection) {
      clCurrentItem->setConnection(newConnection);
      core::SettingsManager::instance().removeCluster(con);
      core::SettingsManager::instance().addCluster(newConnection);

      delete clCurrentItem;
      addCluster(newConnection);
    }
  }

  SentinelConnectionListWidgetItemContainer* sentCurrentItem = dynamic_cast<SentinelConnectionListWidgetItemContainer*>(qitem);  // +
  if (sentCurrentItem) {
    core::ISentinelSettingsBaseSPtr con = sentCurrentItem->connection();
    SentinelDialog dlg(this, con->clone());
    int result = dlg.exec();
    core::ISentinelSettingsBaseSPtr newConnection = dlg.connection();
    if (result == QDialog::Accepted && newConnection) {
      sentCurrentItem->setConnection(newConnection);
      core::SettingsManager::instance().removeSentinel(con);
      core::SettingsManager::instance().addSentinel(newConnection);

      delete sentCurrentItem;
      addSentinel(newConnection);
    }
  }
}

void ConnectionsDialog::connectionSelectChange() {
  bool isEnable = selectedConnection() != nullptr || selectedCluster() != nullptr;
  acButton_->setEnabled(isEnable);
}

/**
 * @brief This function is called when user clicks on "Connect" button.
 */
void ConnectionsDialog::accept() {
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
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if(!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::addCluster(core::IClusterSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->path();
  core::IConnectionSettingsBase::connection_path_t dir(path.directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::root()) {
    ClusterConnectionListWidgetItem* item = new ClusterConnectionListWidgetItem(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if(!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    ClusterConnectionListWidgetItem* item = new ClusterConnectionListWidgetItem(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

void ConnectionsDialog::addSentinel(core::ISentinelSettingsBaseSPtr con) {
  core::IConnectionSettingsBase::connection_path_t path = con->path();
  core::IConnectionSettingsBase::connection_path_t dir(path.directory());
  if (dir == core::IConnectionSettingsBase::connection_path_t::root()) {
    SentinelConnectionListWidgetItemContainer* item = new SentinelConnectionListWidgetItemContainer(con, nullptr);
    listWidget_->addTopLevelItem(item);
  } else {
    DirectoryListWidgetItem* dirItem = findFolderByPath(dir);
    if(!dirItem) {
      dirItem = new DirectoryListWidgetItem(dir);
    }

    SentinelConnectionListWidgetItemContainer* item = new SentinelConnectionListWidgetItemContainer(con, dirItem);
    dirItem->addChild(item);
    listWidget_->addTopLevelItem(dirItem);
  }
}

DirectoryListWidgetItem* ConnectionsDialog::findFolderByPath(const core::IConnectionSettingsBase::connection_path_t& path) const {
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
