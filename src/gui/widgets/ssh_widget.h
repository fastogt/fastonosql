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

#include <QWidget>

#include <fastonosql/core/ssh_info.h>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;

namespace fastonosql {
namespace gui {

class HostPortWidget;
class IPathWidget;

class SSHWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SSHWidget(QWidget* parent = Q_NULLPTR);

  bool isValidSSHInfo() const;

  bool isSSHChecked() const;
  void setSSHChecked(bool checked);

  bool isSSHEnabled() const;
  void setSSHEnabled(bool enabled);

  core::SSHInfo info() const;
  void setInfo(const core::SSHInfo& info);

  core::SSHInfo::AuthenticationMethod selectedAuthMethod() const;

 private Q_SLOTS:
  void securityChange(int index);
  void sshSupportStateChange(int value);
  void publicKeyStateChange(int value);
  void togglePasswordEchoMode();
  void togglePassphraseEchoMode();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QCheckBox* use_ssh_;
  QWidget* use_ssh_widget_;

  HostPortWidget* sshhost_widget_;
  QCheckBox* use_public_key_;
  IPathWidget* public_key_widget_;
  IPathWidget* private_key_widget_;

  QLabel* ssh_address_label_;
  QLabel* ssh_passphrase_label_;
  QLabel* ssh_user_name_label_;
  QLineEdit* user_name_;
  QLabel* ssh_auth_method_label_;
  QLabel* password_label_;

  QComboBox* security_;
  QLineEdit* password_box_;
  QPushButton* password_echo_mode_button_;
  QLineEdit* passphrase_box_;
  QPushButton* passphrase_echo_mode_button_;
};

}  // namespace gui
}  // namespace fastonosql
