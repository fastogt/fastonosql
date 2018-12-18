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

#include "gui/dialogs/base_dialog.h"

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr

class QLabel;  // lines 28-28

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace gui {

class ConnectionDiagnosticDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

 private Q_SLOTS:
  void connectionResultReady(bool suc, qint64 exec_mstime, const QString& result_text);

 protected:
  ConnectionDiagnosticDialog(const QString& title,
                             proxy::IConnectionSettingsBaseSPtr connection,
                             QWidget* parent = Q_NULLPTR);

  void showEvent(QShowEvent* e) override;

 private:
  void startTestConnection(proxy::IConnectionSettingsBaseSPtr connection);

  common::qt::gui::GlassWidget* glass_widget_;
  QLabel* execute_time_label_;
  QLabel* status_label_;
  QLabel* icon_label_;
};

}  // namespace gui
}  // namespace fastonosql
