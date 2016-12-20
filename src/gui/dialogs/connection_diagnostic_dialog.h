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

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr

class QLabel;  // lines 28-28
class QShowEvent;
class QWidget;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}  // lines 33-33

namespace fastonosql {
namespace gui {

class ConnectionDiagnosticDialog : public QDialog {
  Q_OBJECT
 public:
  ConnectionDiagnosticDialog(QWidget* parent, proxy::IConnectionSettingsBaseSPtr connection);

 private Q_SLOTS:
  void connectionResult(bool suc, qint64 mstimeExecute, const QString& resultText);

 protected:
  virtual void showEvent(QShowEvent* e) override;

 private:
  void startTestConnection(proxy::IConnectionSettingsBaseSPtr connection);

  common::qt::gui::GlassWidget* glassWidget_;
  QLabel* executeTimeLabel_;
  QLabel* statusLabel_;
  QLabel* iconLabel_;
};

}  // namespace gui
}  // namespace fastonosql
