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

#include "gui/widgets/ssh_widget.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QEvent>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "gui/widgets/host_port_widget.h"

#include "translations/global.h"

namespace {
const QString trSelectPrivateKey = QObject::tr("Select private key file");
const QString trPrivateKeyFiles = QObject::tr("Private key files (*.*)");
const QString trSelectPublicKey = QObject::tr("Select public key file");
const QString trPublicKeyFiles = QObject::tr("Public key files (*.*)");
}

namespace fastonosql {
namespace gui {

SSHWidget::SSHWidget(QWidget* parent) : QWidget(parent) {
  useSsh_ = new QCheckBox;
  userName_ = new QLineEdit;

  sshHostWidget_ = new HostPortWidget;
  QLayout* host_layout = sshHostWidget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);

  passwordLabel_ = new QLabel;
  sshPrivateKeyLabel_ = new QLabel;
  sshPublicKeyLabel_ = new QLabel;
  sshPassphraseLabel_ = new QLabel;
  sshAddressLabel_ = new QLabel;
  sshUserNameLabel_ = new QLabel;
  sshAuthMethodLabel_ = new QLabel;

  security_ = new QComboBox;
  security_->addItems(QStringList() << translations::trPassword
                                    << translations::trPublicPrivateKey);

  typedef void (QComboBox::*ind)(const QString&);
  VERIFY(connect(security_, static_cast<ind>(&QComboBox::currentIndexChanged), this,
                 &SSHWidget::securityChange));

  passwordBox_ = new QLineEdit;
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this,
                 &SSHWidget::togglePasswordEchoMode));

  privateKeyBox_ = new QLineEdit;
  publicKeyBox_ = new QLineEdit;
  passphraseBox_ = new QLineEdit;

  passphraseBox_->setEchoMode(QLineEdit::Password);
  passphraseEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passphraseEchoModeButton_, &QPushButton::clicked, this,
                 &SSHWidget::togglePassphraseEchoMode));

  useSshWidget_ = new QWidget;

  QGridLayout* sshWidgetLayout = new QGridLayout;
  sshWidgetLayout->setAlignment(Qt::AlignTop);
  sshWidgetLayout->setColumnStretch(1, 1);
  sshWidgetLayout->addWidget(sshAddressLabel_, 1, 0);
  sshWidgetLayout->addWidget(sshHostWidget_, 1, 1, 1, 2);
  sshWidgetLayout->addWidget(sshUserNameLabel_, 2, 0);
  sshWidgetLayout->addWidget(userName_, 2, 1, 1, 2);
  sshWidgetLayout->addWidget(sshAuthMethodLabel_, 4, 0);
  sshWidgetLayout->addWidget(security_, 4, 1, 1, 2);
  sshWidgetLayout->addWidget(passwordLabel_, 5, 0);
  sshWidgetLayout->addWidget(passwordBox_, 5, 1);
  sshWidgetLayout->addWidget(passwordEchoModeButton_, 5, 2);

  sshWidgetLayout->addWidget(sshPrivateKeyLabel_, 7, 0);
  sshWidgetLayout->addWidget(privateKeyBox_, 7, 1);
  selectPrivateFileButton_ = new QPushButton("...");
  selectPrivateFileButton_->setFixedSize(26, 26);
  sshWidgetLayout->addWidget(selectPrivateFileButton_, 7, 2);

  sshWidgetLayout->addWidget(sshPublicKeyLabel_, 8, 0);
  sshWidgetLayout->addWidget(publicKeyBox_, 8, 1);
  selectPublicFileButton_ = new QPushButton("...");
  selectPublicFileButton_->setFixedSize(26, 26);
  sshWidgetLayout->addWidget(selectPublicFileButton_, 8, 2);

  sshWidgetLayout->addWidget(sshPassphraseLabel_, 9, 0);
  sshWidgetLayout->addWidget(passphraseBox_, 9, 1);
  sshWidgetLayout->addWidget(passphraseEchoModeButton_, 9, 2);
  useSshWidget_->setLayout(sshWidgetLayout);

  VERIFY(
      connect(selectPrivateFileButton_, &QPushButton::clicked, this, &SSHWidget::setPrivateFile));
  VERIFY(connect(selectPublicFileButton_, &QPushButton::clicked, this, &SSHWidget::setPublicFile));
  VERIFY(connect(useSsh_, &QCheckBox::stateChanged, this, &SSHWidget::sshSupportStateChange));

  QVBoxLayout* sshLayout = new QVBoxLayout;
  sshLayout->addWidget(useSsh_);
  sshLayout->addWidget(useSshWidget_);
  setLayout(sshLayout);

  // sync controls
  useSshWidget_->setEnabled(false);
  securityChange(security_->currentText());
  retranslateUi();
}

bool SSHWidget::isSSHChecked() const {
  return useSsh_->isChecked();
}

void SSHWidget::setSSHChecked(bool checked) {
  useSsh_->setChecked(checked);
}

bool SSHWidget::isSSHEnabled() const {
  return useSsh_->isEnabled();
}

void SSHWidget::setSSHEnabled(bool enabled) {
  useSsh_->setEnabled(enabled);
}

core::SSHInfo SSHWidget::info() const {
  core::SSHInfo info;
  info.host = sshHostWidget_->host();
  info.user_name = common::ConvertToString(userName_->text());
  info.password = common::ConvertToString(passwordBox_->text());
  info.public_key = common::ConvertToString(publicKeyBox_->text());
  info.private_key = common::ConvertToString(privateKeyBox_->text());
  info.passphrase = common::ConvertToString(passphraseBox_->text());
  info.current_method = selectedAuthMethod();

  return info;
}

void SSHWidget::setInfo(const core::SSHInfo& info) {
  bool checked = info.IsValid();
  useSsh_->setChecked(checked);
  common::net::HostAndPort host = info.host;
  sshHostWidget_->setHost(host);
  userName_->setText(common::ConvertFromString<QString>(info.user_name));
  if (info.AuthMethod() == core::SSHInfo::PUBLICKEY) {
    security_->setCurrentText(translations::trPublicPrivateKey);
  } else {
    security_->setCurrentText(translations::trPassword);
  }
  passwordBox_->setText(common::ConvertFromString<QString>(info.password));
  privateKeyBox_->setText(common::ConvertFromString<QString>(info.private_key));
  publicKeyBox_->setText(common::ConvertFromString<QString>(info.public_key));
  passphraseBox_->setText(common::ConvertFromString<QString>(info.passphrase));
}

core::SSHInfo::SupportedAuthenticationMetods SSHWidget::selectedAuthMethod() const {
  if (security_->currentText() == translations::trPublicPrivateKey) {
    return core::SSHInfo::PUBLICKEY;
  }

  return core::SSHInfo::PASSWORD;
}

void SSHWidget::securityChange(const QString& text) {
  bool isKey = text == translations::trPublicPrivateKey;
  sshPrivateKeyLabel_->setVisible(isKey);
  privateKeyBox_->setVisible(isKey);
  selectPrivateFileButton_->setVisible(isKey);
  sshPublicKeyLabel_->setVisible(isKey);
  publicKeyBox_->setVisible(isKey);
  selectPublicFileButton_->setVisible(isKey);
  sshPassphraseLabel_->setVisible(isKey);
  passphraseBox_->setVisible(isKey);
  passphraseEchoModeButton_->setVisible(isKey);

  passwordBox_->setVisible(!isKey);
  passwordLabel_->setVisible(!isKey);
  passwordEchoModeButton_->setVisible(!isKey);
}

void SSHWidget::sshSupportStateChange(int value) {
  useSshWidget_->setEnabled(value);
}

void SSHWidget::togglePasswordEchoMode() {
  bool isPassword = passwordBox_->echoMode() == QLineEdit::Password;
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void SSHWidget::togglePassphraseEchoMode() {
  bool isPassword = passphraseBox_->echoMode() == QLineEdit::Password;
  passphraseBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passphraseEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void SSHWidget::setPrivateFile() {
  QString filepath = QFileDialog::getOpenFileName(this, trSelectPrivateKey, privateKeyBox_->text(),
                                                  trPrivateKeyFiles);
  if (filepath.isNull()) {
    return;
  }

  privateKeyBox_->setText(filepath);
}

void SSHWidget::setPublicFile() {
  QString filepath = QFileDialog::getOpenFileName(this, trSelectPublicKey, publicKeyBox_->text(),
                                                  trPublicKeyFiles);
  if (filepath.isNull()) {
    return;
  }

  publicKeyBox_->setText(filepath);
}

void SSHWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void SSHWidget::retranslateUi() {
  useSsh_->setText(tr("Use SSH tunnel"));
  passwordLabel_->setText(tr("User Password:"));
  sshPrivateKeyLabel_->setText(tr("Private key:"));
  sshPublicKeyLabel_->setText(tr("Public key:"));
  sshPassphraseLabel_->setText(tr("Passphrase:"));
  sshAddressLabel_->setText(tr("SSH Address:"));
  sshUserNameLabel_->setText(tr("SSH User Name:"));
  sshAuthMethodLabel_->setText(tr("SSH Auth Method:"));
}

}  // namespace gui
}  // namespace fastonosql
