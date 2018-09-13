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

#pragma once

#include <QDialog>

#include "proxy/connection_settings/iconnection_settings.h"

#if defined(PRO_VERSION)
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#endif

class QTreeWidget;
class QTreeWidgetItem;

namespace fastonosql {
namespace gui {

#if defined(PRO_VERSION)
class ClusterConnectionListWidgetItemContainer;
class SentinelConnectionListWidgetItemContainer;
#endif
class ConnectionListWidgetItem;
class DirectoryListWidgetItem;

class ConnectionsDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480 };

  explicit ConnectionsDialog(QWidget* parent = Q_NULLPTR);

  proxy::IConnectionSettingsBaseSPtr selectedConnection() const;
#if defined(PRO_VERSION)
  proxy::ISentinelSettingsBaseSPtr selectedSentinel() const;
  proxy::IClusterSettingsBaseSPtr selectedCluster() const;
#endif

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
#if defined(PRO_VERSION)
  void editCluster(ClusterConnectionListWidgetItemContainer* clusterItem, bool remove_origin);
  void editSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem, bool remove_origin);
#endif

  void removeConnection(ConnectionListWidgetItem* connectionItem);
#if defined(PRO_VERSION)
  void removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);
#endif

  void retranslateUi();

  void addConnection(proxy::IConnectionSettingsBaseSPtr con);
#if defined(PRO_VERSION)
  void addCluster(proxy::IClusterSettingsBaseSPtr con);
  void addSentinel(proxy::ISentinelSettingsBaseSPtr con);
#endif
  DirectoryListWidgetItem* findFolderByPath(const proxy::connection_path_t& path) const;

  QTreeWidget* list_widget_;
  QPushButton* ok_button_;
};

}  // namespace gui
}  // namespace fastonosql
