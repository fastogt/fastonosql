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

#pragma once

#include <QDialog>

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/iconnection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace fastonosql {
namespace gui {

class ClusterConnectionListWidgetItemContainer;
class ConnectionListWidgetItem;
class DirectoryListWidgetItem;
class SentinelConnectionListWidgetItemContainer;

class ConnectionsDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480 };

  explicit ConnectionsDialog(QWidget* parent = Q_NULLPTR);

  proxy::IConnectionSettingsBaseSPtr selectedConnection() const;
  proxy::ISentinelSettingsBaseSPtr selectedSentinel() const;
  proxy::IClusterSettingsBaseSPtr selectedCluster() const;

 private Q_SLOTS:
  virtual void accept() override;
  void add();
  void addCls();
  void addSent();
  void remove();
  void clone();
  void edit();
  void itemSelectionChange();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void editItem(QTreeWidgetItem* qitem, bool remove_origin);
  void editConnection(ConnectionListWidgetItem* connectionItem, bool remove_origin);
  void editCluster(ClusterConnectionListWidgetItemContainer* clusterItem, bool remove_origin);
  void editSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem, bool remove_origin);

  void removeConnection(ConnectionListWidgetItem* connectionItem);
  void removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);

  void retranslateUi();

  void addConnection(proxy::IConnectionSettingsBaseSPtr con);
  void addCluster(proxy::IClusterSettingsBaseSPtr con);
  void addSentinel(proxy::ISentinelSettingsBaseSPtr con);
  DirectoryListWidgetItem* findFolderByPath(const proxy::connection_path_t& path) const;

  QTreeWidget* listWidget_;
  QPushButton* acButton_;
};
}  // namespace gui
}  // namespace fastonosql
