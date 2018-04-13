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
namespace {
core::SSHInfo::SupportedAuthenticationMethods ConvertIndexToSSHMethod(int index) {
  CHECK(index != -1) << "Impossible to select nothing in combobox.";
  return static_cast<core::SSHInfo::SupportedAuthenticationMethods>(index + 1);  // because 0 -> UNKNOWN
}
}  // namespace
namespace gui {

SSHWidget::SSHWidget(QWidget* parent) : QWidget(parent) {
  use_ssh_ = new QCheckBox;
  user_name_ = new QLineEdit;

  sshhost_widget_ = new HostPortWidget;
  QLayout* host_layout = sshhost_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);

  private_key_widget_ = new FilePathWidget(trPrivateKey, trPrivateKeyFiles, trSelectPrivateKey);
  QLayout* pub_layout = private_key_widget_->layout();
  pub_layout->setContentsMargins(0, 0, 0, 0);

  use_public_key_ = new QCheckBox;

  public_key_widget_ = new FilePathWidget(trPublicKey, trPublicKeyFiles, trSelectPublicKey);
  QLayout* priv_layout = public_key_widget_->layout();
  priv_layout->setContentsMargins(0, 0, 0, 0);

  password_label_ = new QLabel;
  ssh_passphrase_label_ = new QLabel;
  ssh_address_label_ = new QLabel;
  ssh_user_name_label_ = new QLabel;
  ssh_auth_method_label_ = new QLabel;

  security_ = new QComboBox;
  security_->addItem(translations::trPublicPrivateKey, core::SSHInfo::PUBLICKEY);
  security_->addItem(translations::trAskPassword, core::SSHInfo::ASK_PASSWORD);
  security_->addItem(translations::trPassword, core::SSHInfo::PASSWORD);

  typedef void (QComboBox::*ind)(int index);
  VERIFY(connect(security_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &SSHWidget::securityChange));

  password_box_ = new QLineEdit;
  password_box_->setEchoMode(QLineEdit::Password);
  password_echo_mode_button_ = new QPushButton(translations::trShow);
  VERIFY(connect(password_echo_mode_button_, &QPushButton::clicked, this, &SSHWidget::togglePasswordEchoMode));

  passphrase_box_ = new QLineEdit;

  passphrase_box_->setEchoMode(QLineEdit::Password);
  passphrase_echo_mode_button_ = new QPushButton(translations::trShow);
  VERIFY(connect(passphrase_echo_mode_button_, &QPushButton::clicked, this, &SSHWidget::togglePassphraseEchoMode));

  use_ssh_widget_ = new QWidget;

  QVBoxLayout* sshWidgetLayout = new QVBoxLayout;
  sshWidgetLayout->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* ssh = new QHBoxLayout;
  ssh->addWidget(ssh_address_label_);
  ssh->addWidget(sshhost_widget_);
  sshWidgetLayout->addLayout(ssh);

  QHBoxLayout* ssh_user = new QHBoxLayout;
  ssh_user->addWidget(ssh_user_name_label_);
  ssh_user->addWidget(user_name_);
  sshWidgetLayout->addLayout(ssh_user);

  QHBoxLayout* ssh_auth = new QHBoxLayout;
  ssh_auth->addWidget(ssh_auth_method_label_);
  ssh_auth->addWidget(security_);
  sshWidgetLayout->addLayout(ssh_auth);

  QHBoxLayout* pass = new QHBoxLayout;
  pass->addWidget(password_label_);
  pass->addWidget(password_box_);
  pass->addWidget(password_echo_mode_button_);
  sshWidgetLayout->addLayout(pass);

  sshWidgetLayout->addWidget(private_key_widget_);

  QHBoxLayout* priv = new QHBoxLayout;
  priv->addWidget(public_key_widget_);
  priv->addWidget(use_public_key_);
  sshWidgetLayout->addLayout(priv);

  QHBoxLayout* passp = new QHBoxLayout;
  passp->addWidget(ssh_passphrase_label_);
  passp->addWidget(passphrase_box_);
  passp->addWidget(passphrase_echo_mode_button_);
  sshWidgetLayout->addLayout(passp);
  use_ssh_widget_->setLayout(sshWidgetLayout);

  VERIFY(connect(use_ssh_, &QCheckBox::stateChanged, this, &SSHWidget::sshSupportStateChange));
  VERIFY(connect(use_public_key_, &QCheckBox::stateChanged, this, &SSHWidget::publicKeyStateChange));

  QVBoxLayout* sshLayout = new QVBoxLayout;
  sshLayout->addWidget(use_ssh_);
  sshLayout->addWidget(use_ssh_widget_);
  setLayout(sshLayout);

  // sync controls
  use_ssh_widget_->setEnabled(false);
  securityChange(security_->currentIndex());
  publicKeyStateChange(0);
  retranslateUi();
}

bool SSHWidget::isSSHChecked() const {
  return use_ssh_->isChecked();
}

void SSHWidget::setSSHChecked(bool checked) {
  use_ssh_->setChecked(checked);
}

bool SSHWidget::isSSHEnabled() const {
  return use_ssh_->isEnabled();
}

void SSHWidget::setSSHEnabled(bool enabled) {
  use_ssh_->setEnabled(enabled);
}

core::SSHInfo SSHWidget::info() const {
  core::SSHInfo info;
  info.host = sshhost_widget_->host();
  info.current_method = selectedAuthMethod();  // should be first, setpassword dep on it
  info.user_name = common::ConvertToString(user_name_->text());
  info.SetPassword(common::ConvertToString(password_box_->text()));
  info.key.public_key = common::ConvertToString(public_key_widget_->path());
  info.key.private_key = common::ConvertToString(private_key_widget_->path());
  info.key.use_public_key = use_public_key_->isChecked();
  info.passphrase = common::ConvertToString(passphrase_box_->text());

  return info;
}

void SSHWidget::setInfo(const core::SSHInfo& info) {
  bool checked = info.IsValid();
  use_ssh_->setChecked(checked);
  common::net::HostAndPort host = info.host;
  sshhost_widget_->setHost(host);
  QString quser_name;
  common::ConvertFromString(info.user_name, &quser_name);
  user_name_->setText(quser_name);
  core::SSHInfo::SupportedAuthenticationMethods method = info.GetAuthMethod();
  if (method != core::SSHInfo::UNKNOWN) {
    security_->setCurrentIndex(method - 1);  //
  }

  QString qpassword;
  common::ConvertFromString(info.GetRuntimePassword(), &qpassword);
  password_box_->setText(qpassword);

  QString qprivate_key;
  common::ConvertFromString(info.key.private_key, &qprivate_key);
  private_key_widget_->setPath(qprivate_key);

  QString qpublic_key;
  common::ConvertFromString(info.key.public_key, &qpublic_key);
  public_key_widget_->setPath(qpublic_key);

  use_public_key_->setChecked(info.key.use_public_key);

  QString qpassphrase;
  common::ConvertFromString(info.passphrase, &qpassphrase);
  passphrase_box_->setText(qpassphrase);
}

core::SSHInfo::SupportedAuthenticationMethods SSHWidget::selectedAuthMethod() const {
  int index = security_->currentIndex();
  return ConvertIndexToSSHMethod(index);
}

void SSHWidget::securityChange(int index) {
  core::SSHInfo::SupportedAuthenticationMethods method = ConvertIndexToSSHMethod(index);
  bool is_public_key = method == core::SSHInfo::PUBLICKEY;
  private_key_widget_->setVisible(is_public_key);
  use_public_key_->setVisible(is_public_key);
  public_key_widget_->setVisible(is_public_key);
  ssh_passphrase_label_->setVisible(is_public_key);
  passphrase_box_->setVisible(is_public_key);
  passphrase_echo_mode_button_->setVisible(is_public_key);

  password_box_->setVisible(!is_public_key);
  password_label_->setVisible(!is_public_key);
  password_echo_mode_button_->setVisible(!is_public_key);

  if (method == core::SSHInfo::ASK_PASSWORD) {
    password_box_->clear();
  }
  // enabled onli if password
  password_echo_mode_button_->setEnabled(method == core::SSHInfo::PASSWORD);
  password_box_->setEnabled(method == core::SSHInfo::PASSWORD);
}

void SSHWidget::sshSupportStateChange(int value) {
  use_ssh_widget_->setEnabled(value);
}

void SSHWidget::publicKeyStateChange(int value) {
  public_key_widget_->setEnabled(value);
}

void SSHWidget::togglePasswordEchoMode() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_box_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  password_echo_mode_button_->setText(isPassword ? translations::trHide : translations::trShow);
}

void SSHWidget::togglePassphraseEchoMode() {
  bool isPassword = passphrase_box_->echoMode() == QLineEdit::Password;
  passphrase_box_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passphrase_echo_mode_button_->setText(isPassword ? translations::trHide : translations::trShow);
}

bool SSHWidget::isValidSSHInfo() const {
  if (use_ssh_->isChecked()) {
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
  use_ssh_->setText(tr("Use SSH tunnel"));
  use_public_key_->setText(tr("Use public key"));
  password_label_->setText(tr("User Password:"));
  ssh_passphrase_label_->setText(tr("Passphrase:"));
  ssh_address_label_->setText(tr("SSH Address:"));
  ssh_user_name_label_->setText(tr("SSH User Name:"));
  ssh_auth_method_label_->setText(tr("SSH Auth Method:"));
}

}  // namespace gui
}  // namespace fastonosql
