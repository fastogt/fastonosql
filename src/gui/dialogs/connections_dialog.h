/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QDialog>

#include "core/cluster_connection_settings.h"
#include "core/connection_settings.h"
#include "core/sentinel_connection_settings.h"

class QTreeWidget;

namespace fastonosql {
namespace gui {
class ClusterConnectionListWidgetItemContainer;
}
}  // lines 31-31
namespace fastonosql {
namespace gui {
class ConnectionListWidgetItem;
}
}  // lines 33-33
namespace fastonosql {
namespace gui {
class DirectoryListWidgetItem;
}
}  // lines 30-30
namespace fastonosql {
namespace gui {
class SentinelConnectionListWidgetItemContainer;
}
}  // lines 32-32

namespace fastonosql {
namespace gui {

class ConnectionsDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480 };

  explicit ConnectionsDialog(QWidget* parent = 0);

  core::IConnectionSettingsBaseSPtr selectedConnection() const;
  core::ISentinelSettingsBaseSPtr selectedSentinel() const;
  core::IClusterSettingsBaseSPtr selectedCluster() const;

  virtual void accept();

 private Q_SLOTS:
  void add();
  void addCls();
  void addSent();
  void remove();
  void edit();
  void itemSelectionChange();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  void editConnection(ConnectionListWidgetItem* connectionItem);
  void editCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void editSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);

  void removeConnection(ConnectionListWidgetItem* connectionItem);
  void removeCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void removeSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);

  void retranslateUi();

  void addConnection(core::IConnectionSettingsBaseSPtr con);
  void addCluster(core::IClusterSettingsBaseSPtr con);
  void addSentinel(core::ISentinelSettingsBaseSPtr con);
  DirectoryListWidgetItem* findFolderByPath(
      const core::IConnectionSettings::connection_path_t& path) const;

  QTreeWidget* listWidget_;
  QPushButton* acButton_;
};

}  // namespace gui
}  // namespace fastonosql
