/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "core/ssh_info.h"

class QLabel;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QPushButton;

namespace fastonosql {
namespace gui {

class HostPortWidget;
class PathWidget;

class SSHWidget : public QWidget {
  Q_OBJECT
 public:
  explicit SSHWidget(QWidget* parent = 0);

  bool isValidSSHInfo() const;

  bool isSSHChecked() const;
  void setSSHChecked(bool checked);

  bool isSSHEnabled() const;
  void setSSHEnabled(bool enabled);

  core::SSHInfo info() const;
  void setInfo(const core::SSHInfo& info);

  core::SSHInfo::SupportedAuthenticationMetods selectedAuthMethod() const;

 private Q_SLOTS:
  void securityChange(const QString& text);
  void sshSupportStateChange(int value);
  void togglePasswordEchoMode();
  void togglePassphraseEchoMode();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QCheckBox* useSsh_;
  QWidget* useSshWidget_;

  HostPortWidget* sshHostWidget_;
  PathWidget* publicKeyWidget_;
  PathWidget* privateKeyWidget_;

  QLabel* sshAddressLabel_;
  QLabel* sshPassphraseLabel_;
  QLabel* sshUserNameLabel_;
  QLineEdit* userName_;
  QLabel* sshAuthMethodLabel_;
  QLabel* passwordLabel_;

  QComboBox* security_;
  QLineEdit* passwordBox_;
  QPushButton* passwordEchoModeButton_;
  QLineEdit* passphraseBox_;
  QPushButton* passphraseEchoModeButton_;
};

}  // namespace gui
}  // namespace fastonosql
