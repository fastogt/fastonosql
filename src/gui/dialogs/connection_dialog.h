/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QSpinBox;

#include "core/connection_settings.h"

namespace fastonosql {
class ConnectionDialog
  : public QDialog {
  Q_OBJECT

 public:
  ConnectionDialog(QWidget* parent, IConnectionSettingsBase* connection = NULL,
                   const std::vector<connectionTypes>& availibleTypes = std::vector<connectionTypes>()); //get ownerships connection

  IConnectionSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  virtual void accept();

 private Q_SLOTS:
  void typeConnectionChange(int index);
  void loggingStateChange(int value);
  void securityChange(const QString& val);
  void sshSupportStateChange(int value);
  void togglePasswordEchoMode();
  void togglePassphraseEchoMode();
  void setPrivateFile();
  void testConnection();

 protected:
  virtual void changeEvent(QEvent* );

 private:
  void retranslateUi();
  bool validateAndApply();
  SSHInfo::SupportedAuthenticationMetods selectedAuthMethod() const;
  void updateSshControls(bool isValidType);

  IConnectionSettingsBaseSPtr connection_;
  QLineEdit* connectionName_;
  QComboBox* typeConnection_;
  QCheckBox* logging_;
  QSpinBox* loggingMsec_;
  QLineEdit* commandLine_;

  QPushButton* testButton_;
  QDialogButtonBox *buttonBox_;

  QCheckBox* useSsh_;

  QWidget* useSshWidget_;
  QLineEdit* sshHostName_;
  QLineEdit* sshPort_;

  QLabel* sshAddressLabel_;
  QLabel* sshPassphraseLabel_;
  QLabel* sshUserNameLabel_;
  QLineEdit* userName_;
  QLabel* sshAuthMethodLabel_;
  QLabel* passwordLabel_;
  QLabel* sshPrivateKeyLabel_;

  QComboBox* security_;
  QLineEdit* passwordBox_;
  QPushButton* passwordEchoModeButton_;
  QLineEdit* privateKeyBox_;
  QLineEdit* passphraseBox_;
  QPushButton* passphraseEchoModeButton_;
  QPushButton* selectPrivateFileButton_;
};

}
