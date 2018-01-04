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

#include "gui/widgets/ssh_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>

#include <common/qt/convert2string.h>

#include "gui/widgets/host_port_widget.h"
#include "gui/widgets/path_widget.h"

#include "translations/global.h"

namespace {
const QString trSelectPrivateKey = QObject::tr("Select private key file");
const QString trPrivateKeyFiles = QObject::tr("Private key files (*)");
const QString trSelectPublicKey = QObject::tr("Select public key file");
const QString trPublicKeyFiles = QObject::tr("Public key files (*)");
const QString trPrivateKey = QObject::tr("Private key:");
const QString trPublicKey = QObject::tr("Public key:");
}  // namespace

namespace fastonosql {
namespace gui {

SSHWidget::SSHWidget(QWidget* parent) : QWidget(parent) {
  useSsh_ = new QCheckBox;
  userName_ = new QLineEdit;

  sshhost_widget_ = new HostPortWidget;
  QLayout* host_layout = sshhost_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);

  privateKeyWidget_ = new FilePathWidget(trPrivateKey, trPrivateKeyFiles, trSelectPrivateKey);
  QLayout* pub_layout = privateKeyWidget_->layout();
  pub_layout->setContentsMargins(0, 0, 0, 0);

  use_public_key_ = new QCheckBox;

  publicKeyWidget_ = new FilePathWidget(trPublicKey, trPublicKeyFiles, trSelectPublicKey);
  QLayout* priv_layout = publicKeyWidget_->layout();
  priv_layout->setContentsMargins(0, 0, 0, 0);

  passwordLabel_ = new QLabel;
  sshPassphraseLabel_ = new QLabel;
  sshAddressLabel_ = new QLabel;
  sshUserNameLabel_ = new QLabel;
  sshAuthMethodLabel_ = new QLabel;

  security_ = new QComboBox;
  security_->addItems(QStringList() << translations::trPassword << translations::trPublicPrivateKey);

  typedef void (QComboBox::*ind)(const QString&);
  VERIFY(connect(security_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &SSHWidget::securityChange));

  passwordBox_ = new QLineEdit;
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this, &SSHWidget::togglePasswordEchoMode));

  passphraseBox_ = new QLineEdit;

  passphraseBox_->setEchoMode(QLineEdit::Password);
  passphraseEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passphraseEchoModeButton_, &QPushButton::clicked, this, &SSHWidget::togglePassphraseEchoMode));

  useSshWidget_ = new QWidget;

  QVBoxLayout* sshWidgetLayout = new QVBoxLayout;
  sshWidgetLayout->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* ssh = new QHBoxLayout;
  ssh->addWidget(sshAddressLabel_);
  ssh->addWidget(sshhost_widget_);
  sshWidgetLayout->addLayout(ssh);

  QHBoxLayout* ssh_user = new QHBoxLayout;
  ssh_user->addWidget(sshUserNameLabel_);
  ssh_user->addWidget(userName_);
  sshWidgetLayout->addLayout(ssh_user);

  QHBoxLayout* ssh_auth = new QHBoxLayout;
  ssh_auth->addWidget(sshAuthMethodLabel_);
  ssh_auth->addWidget(security_);
  sshWidgetLayout->addLayout(ssh_auth);

  QHBoxLayout* pass = new QHBoxLayout;
  pass->addWidget(passwordLabel_);
  pass->addWidget(passwordBox_);
  pass->addWidget(passwordEchoModeButton_);
  sshWidgetLayout->addLayout(pass);

  sshWidgetLayout->addWidget(privateKeyWidget_);

  QHBoxLayout* priv = new QHBoxLayout;
  priv->addWidget(publicKeyWidget_);
  priv->addWidget(use_public_key_);
  sshWidgetLayout->addLayout(priv);

  QHBoxLayout* passp = new QHBoxLayout;
  passp->addWidget(sshPassphraseLabel_);
  passp->addWidget(passphraseBox_);
  passp->addWidget(passphraseEchoModeButton_);
  sshWidgetLayout->addLayout(passp);
  useSshWidget_->setLayout(sshWidgetLayout);

  VERIFY(connect(useSsh_, &QCheckBox::stateChanged, this, &SSHWidget::sshSupportStateChange));
  VERIFY(connect(use_public_key_, &QCheckBox::stateChanged, this, &SSHWidget::publicKeyStateChange));

  QVBoxLayout* sshLayout = new QVBoxLayout;
  sshLayout->addWidget(useSsh_);
  sshLayout->addWidget(useSshWidget_);
  setLayout(sshLayout);

  // sync controls
  useSshWidget_->setEnabled(false);
  securityChange(security_->currentText());
  publicKeyStateChange(0);
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
  info.host = sshhost_widget_->host();
  info.user_name = common::ConvertToString(userName_->text());
  info.password = common::ConvertToString(passwordBox_->text());
  info.key.public_key = common::ConvertToString(publicKeyWidget_->path());
  info.key.private_key = common::ConvertToString(privateKeyWidget_->path());
  info.key.use_public_key = use_public_key_->isChecked();
  info.passphrase = common::ConvertToString(passphraseBox_->text());
  info.current_method = selectedAuthMethod();

  return info;
}

void SSHWidget::setInfo(const core::SSHInfo& info) {
  bool checked = info.IsValid();
  useSsh_->setChecked(checked);
  common::net::HostAndPort host = info.host;
  sshhost_widget_->setHost(host);
  QString quser_name;
  common::ConvertFromString(info.user_name, &quser_name);
  userName_->setText(quser_name);
  if (info.GetAuthMethod() == core::SSHInfo::PUBLICKEY) {
    security_->setCurrentText(translations::trPublicPrivateKey);
  } else {
    security_->setCurrentText(translations::trPassword);
  }

  QString qpassword;
  common::ConvertFromString(info.password, &qpassword);
  passwordBox_->setText(qpassword);

  QString qprivate_key;
  common::ConvertFromString(info.key.private_key, &qprivate_key);
  privateKeyWidget_->setPath(qprivate_key);

  QString qpublic_key;
  common::ConvertFromString(info.key.public_key, &qpublic_key);
  publicKeyWidget_->setPath(qpublic_key);

  use_public_key_->setChecked(info.key.use_public_key);

  QString qpassphrase;
  common::ConvertFromString(info.passphrase, &qpassphrase);
  passphraseBox_->setText(qpassphrase);
}

core::SSHInfo::SupportedAuthenticationMetods SSHWidget::selectedAuthMethod() const {
  if (security_->currentText() == translations::trPublicPrivateKey) {
    return core::SSHInfo::PUBLICKEY;
  }

  return core::SSHInfo::PASSWORD;
}

void SSHWidget::securityChange(const QString& text) {
  bool isKey = text == translations::trPublicPrivateKey;
  privateKeyWidget_->setVisible(isKey);
  use_public_key_->setVisible(isKey);
  publicKeyWidget_->setVisible(isKey);
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

void SSHWidget::publicKeyStateChange(int value) {
  publicKeyWidget_->setEnabled(value);
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

bool SSHWidget::isValidSSHInfo() const {
  if (useSsh_->isChecked()) {
    core::SSHInfo inf = info();
    return inf.IsValid();
  }

  return true;
}

void SSHWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void SSHWidget::retranslateUi() {
  useSsh_->setText(tr("Use SSH tunnel"));
  use_public_key_->setText(tr("Use public key"));
  passwordLabel_->setText(tr("User Password:"));
  sshPassphraseLabel_->setText(tr("Passphrase:"));
  sshAddressLabel_->setText(tr("SSH Address:"));
  sshUserNameLabel_->setText(tr("SSH User Name:"));
  sshAuthMethodLabel_->setText(tr("SSH Auth Method:"));
}

}  // namespace gui
}  // namespace fastonosql
