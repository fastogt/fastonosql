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

#pragma once

#include <QDialog>

#include "core/connection_settings/connection_settings.h"
#include "core/connection_settings/cluster_connection_settings.h"
#include "core/connection_settings/sentinel_connection_settings.h"

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

  core::IConnectionSettingsBaseSPtr SelectedConnection() const;
  core::ISentinelSettingsBaseSPtr SelectedSentinel() const;
  core::IClusterSettingsBaseSPtr SelectedCluster() const;

 private Q_SLOTS:
  virtual void accept() override;
  void Add();
  void AddCls();
  void AddSent();
  void Remove();
  void Edit();
  void ItemSelectionChange();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void EditConnection(ConnectionListWidgetItem* connectionItem);
  void EditCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void EditSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);

  void RemoveConnection(ConnectionListWidgetItem* connectionItem);
  void RemoveCluster(ClusterConnectionListWidgetItemContainer* clusterItem);
  void RemoveSentinel(SentinelConnectionListWidgetItemContainer* sentinelItem);

  void RetranslateUi();

  void AddConnection(core::IConnectionSettingsBaseSPtr con);
  void AddCluster(core::IClusterSettingsBaseSPtr con);
  void AddSentinel(core::ISentinelSettingsBaseSPtr con);
  DirectoryListWidgetItem* FindFolderByPath(
      const core::IConnectionSettings::connection_path_t& path) const;

  QTreeWidget* listWidget_;
  QPushButton* acButton_;
};
}  // namespace gui
}  // namespace fastonosql
