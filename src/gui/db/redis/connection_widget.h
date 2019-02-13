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

#include "gui/widgets/connection_base_widget.h"

class QPushButton;
class QRadioButton;
class QGroupBox;

namespace fastonosql {
namespace gui {
class SSHWidget;
class HostPortWidget;
class IPathWidget;

namespace redis {

class ConnectionWidget : public ConnectionBaseWidget {
  Q_OBJECT

 public:
  typedef ConnectionBaseWidget base_class;
  template <typename T, typename... Args>
  friend T* gui::createWidget(Args&&... args);

  void syncControls(proxy::IConnectionSettingsBase* connection) override;
  bool validated() const override;
  bool isValidCredential() const;

 private Q_SLOTS:
  void togglePasswordEchoMode();
  void authStateChange(int state);
  void selectRemoteDBPath(bool checked);
  void selectLocalDBPath(bool checked);
  void secureConnectionChange(bool checked);

 protected:
  explicit ConnectionWidget(QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

 private:
  proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const override;
  QGroupBox* group_box_;
  QRadioButton* remote_;
  QRadioButton* local_;

  HostPortWidget* host_widget_;
  QCheckBox* is_ssl_connection_;
  IPathWidget* path_widget_;

  QCheckBox* use_auth_;
  QLineEdit* password_box_;
  QPushButton* password_echo_mode_button_;

  QLabel* default_db_label_;
  QSpinBox* default_db_num_;

  SSHWidget* ssh_widget_;
};

}  // namespace redis
}  // namespace gui
}  // namespace fastonosql
