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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "gui/dialogs/base_dialog.h"

#include "proxy/connection_settings/iconnection_settings.h"

#if defined(PRO_VERSION)
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#endif

class QTreeWidget;
class QTreeWidgetItem;
class QToolBar;

namespace fastonosql {
namespace gui {

#if defined(PRO_VERSION)
class ClusterConnectionListWidgetItemContainer;
class SentinelConnectionListWidgetItemContainer;
#endif
class ConnectionListWidgetItem;
class DirectoryListWidgetItem;

class ConnectionsDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  enum { min_width = 640, min_height = 480 };

  proxy::IConnectionSettingsBaseSPtr selectedConnection() const;
#if defined(PRO_VERSION)
  proxy::ISentinelSettingsBaseSPtr selectedSentinel() const;
  proxy::IClusterSettingsBaseSPtr selectedCluster() const;
#endif

 private Q_SLOTS:
  void accept() override;
  void addConnectionAction();
  void addClusterAction();
  void addSentinelAction();
  void removeItemAction();
  void cloneItemAction();
  void editItemAction();
  void itemSelectionChange();

 protected:
  explicit ConnectionsDialog(const QString& title, const QIcon& icon, QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private:
  QToolBar* createToolBar();

  void editItem(QTreeWidgetItem* qitem, bool remove_origin);
  void editConnection(ConnectionListWidgetItem* connection_item, bool remove_origin);
  void removeConnection(ConnectionListWidgetItem* connectionItem);
  void addConnection(proxy::IConnectionSettingsBaseSPtr con);

#if defined(PRO_VERSION)
  void editCluster(ClusterConnectionListWidgetItemContainer* cluster_item, bool remove_origin);
  void editSentinel(SentinelConnectionListWidgetItemContainer* sentinel_item, bool remove_origin);
  void removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);
  void addCluster(proxy::IClusterSettingsBaseSPtr con);
  void addSentinel(proxy::ISentinelSettingsBaseSPtr con);
#endif

  DirectoryListWidgetItem* findFolderByPath(const proxy::connection_path_t& path) const;

  QAction* add_connection_action_;
  QAction* add_cluster_action_;
  QAction* add_sentinel_action_;
  QAction* edit_action_;
  QAction* clone_action_;
  QAction* remove_action_;

  QTreeWidget* list_widget_;
  QPushButton* ok_button_;
};

}  // namespace gui
}  // namespace fastonosql
