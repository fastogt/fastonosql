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

#include <vector>  // for vector

#include <QDialog>

#include <common/types.h>  // for time64_t

#include "core/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc
#include "core/connection_settings/icluster_connection_settings.h"
#include "core/server/iserver_info.h"

class QMovie;
class QLabel;
class QTreeWidget;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}

namespace fastonosql {
namespace gui {

class ConnectionListWidgetItemDiscovered;
class DiscoveryClusterDiagnosticDialog : public QDialog {
  Q_OBJECT
 public:
  enum { fix_height = 320, fix_width = 480 };

  DiscoveryClusterDiagnosticDialog(QWidget* parent,
                                   core::IConnectionSettingsBaseSPtr connection,
                                   core::IClusterSettingsBaseSPtr cluster);
  std::vector<fastonosql::gui::ConnectionListWidgetItemDiscovered*> selectedConnections() const;

 private Q_SLOTS:
  void connectionResult(bool suc,
                        qint64 mstimeExecute,
                        const QString& resultText,
                        std::vector<core::ServerDiscoveryClusterInfoSPtr> infos);

 protected:
  virtual void showEvent(QShowEvent* e) override;

 private:
  void testConnection(core::IConnectionSettingsBaseSPtr connection);

  common::qt::gui::GlassWidget* glassWidget_;
  QLabel* executeTimeLabel_;
  QLabel* statusLabel_;
  QTreeWidget* listWidget_;
  QLabel* iconLabel_;
  core::IClusterSettingsBaseSPtr cluster_;
};

}  // namespace gui
}  // namespace fastonosql
