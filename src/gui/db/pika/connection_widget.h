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

#include "gui/widgets/connection_base_widget.h"

class QPushButton;
class QRadioButton;
class QGroupBox;

namespace fastonosql {
namespace gui {
class SSHWidget;
class HostPortWidget;
class IPathWidget;

namespace pika {

class ConnectionWidget : public ConnectionBaseWidget {
  Q_OBJECT
 public:
  explicit ConnectionWidget(QWidget* parent = Q_NULLPTR);

  void syncControls(proxy::IConnectionSettingsBase* connection) override;
  void retranslateUi() override;
  bool validated() const override;
  bool isValidCredential() const;

 private Q_SLOTS:
  void togglePasswordEchoMode();
  void authStateChange(int state);
  void sslStateChange(int state);

 private:
  proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const override;
  HostPortWidget* host_widget_;
  QCheckBox* is_ssl_connection_;

  QCheckBox* use_auth_;
  QLineEdit* password_box_;
  QPushButton* password_echo_mode_button_;

  QLabel* default_db_label_;
  QSpinBox* default_db_num_;

  SSHWidget* ssh_widget_;
};

}  // namespace pika
}  // namespace gui
}  // namespace fastonosql
