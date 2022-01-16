/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <fastonosql/core/connection_types.h>

#include "gui/dialogs/base_dialog.h"

#include "proxy/connection_settings/iconnection_settings.h"

namespace fastonosql {
namespace gui {

class ConnectionBaseWidget;

class ConnectionDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  void setFolderEnabled(bool val);
  proxy::IConnectionSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  void accept() override;

 private Q_SLOTS:
  void testConnection();

 protected:
  ConnectionDialog(core::ConnectionType type, const QString& connection_name, QWidget* parent = Q_NULLPTR);
  explicit ConnectionDialog(proxy::IConnectionSettingsBase* connection,
                            QWidget* parent = Q_NULLPTR);  // take ownerships

  void retranslateUi() override;

 private:
  void init(proxy::IConnectionSettingsBase* connection);

  bool validateAndApply();

  ConnectionBaseWidget* connection_widget_;

  QPushButton* test_button_;

  proxy::IConnectionSettingsBaseSPtr connection_;
};

}  // namespace gui
}  // namespace fastonosql
