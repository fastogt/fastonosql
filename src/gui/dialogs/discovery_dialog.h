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

#include <vector>

#include <QDialog>

#include "core/connection_settings.h"
#include "core/types.h"

class QMovie;
class QLabel;
class QTreeWidget;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}

namespace fastonosql {
namespace gui {

class DiscoveryConnection
  : public QObject {
  Q_OBJECT
 public:
  explicit DiscoveryConnection(IConnectionSettingsBaseSPtr conn, QObject* parent = 0);

 Q_SIGNALS:
  void connectionResult(bool suc, qint64 msTimeExecute, const QString& resultText,
                        std::vector<ServerDiscoveryInfoSPtr> infos);

 public Q_SLOTS:
  void routine();

 private:
  IConnectionSettingsBaseSPtr connection_;
  common::time64_t startTime_;
};

class DiscoveryDiagnosticDialog
  : public QDialog {
  Q_OBJECT
 public:
  enum {
    fix_height = 320,
    fix_width = 480
  };

  DiscoveryDiagnosticDialog(QWidget* parent, IConnectionSettingsBaseSPtr connection,
                            IClusterSettingsBaseSPtr cluster);
  std::vector<IConnectionSettingsBaseSPtr> selectedConnections() const;

 private Q_SLOTS:
  void connectionResult(bool suc, qint64 mstimeExecute, const QString&resultText,
                        std::vector<ServerDiscoveryInfoSPtr> infos);

 protected:
  virtual void showEvent(QShowEvent* e);

 private:
  void testConnection(IConnectionSettingsBaseSPtr connection);

  fasto::qt::gui::GlassWidget* glassWidget_;
  QLabel* executeTimeLabel_;
  QLabel* statusLabel_;
  QTreeWidget* listWidget_;
  QLabel* iconLabel_;
  IClusterSettingsBaseSPtr cluster_;
};

}  // namespace gui
}  // namespace fastonosql
