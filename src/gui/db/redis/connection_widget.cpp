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

#include "gui/db/redis/connection_widget.h"

#include <QCheckBox>
#include <QEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExpValidator>
#include <QSpinBox>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "proxy/db/redis/connection_settings.h"

#include "gui/widgets/host_port_widget.h"
#include "gui/widgets/path_widget.h"
#include "gui/widgets/ssh_widget.h"

#include "translations/global.h"

namespace {
const QString trUnixPath = QObject::tr("Unix socket path:");
const QString trRemote = QObject::tr("Remote");
const QString trLocal = QObject::tr("Local");
const QString trSSL = QObject::tr("SSL");
}  // namespace

namespace fastonosql {
namespace gui {
namespace redis {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionBaseWidget(parent) {
  QVBoxLayout* vbox = new QVBoxLayout;
  groupBox_ = new QGroupBox;
  remote_ = new QRadioButton;
  local_ = new QRadioButton;
  VERIFY(connect(remote_, &QRadioButton::toggled, this, &ConnectionWidget::selectRemoteDBPath));
  VERIFY(connect(local_, &QRadioButton::toggled, this, &ConnectionWidget::selectLocalDBPath));

  isSSLConnection_ = new QCheckBox;
  VERIFY(connect(isSSLConnection_, &QCheckBox::stateChanged, this, &ConnectionWidget::sslStateChange));

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(remote_);
  hbox->addWidget(isSSLConnection_);

  host_widget_ = new HostPortWidget;
  QLayout* host_layout = host_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addLayout(hbox);
  vbox->addWidget(host_widget_);

  pathWidget_ = new FilePathWidget(trUnixPath, trFilter, trCaption);
  QLayout* path_layout = pathWidget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(local_);
  vbox->addWidget(pathWidget_);

  groupBox_->setLayout(vbox);
  addWidget(groupBox_);

  useAuth_ = new QCheckBox;
  VERIFY(connect(useAuth_, &QCheckBox::stateChanged, this, &ConnectionWidget::authStateChange));
  addWidget(useAuth_);

  QHBoxLayout* passwordLayout = new QHBoxLayout;
  passwordBox_ = new QLineEdit;
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this, &ConnectionWidget::togglePasswordEchoMode));
  passwordLayout->addWidget(passwordBox_);
  passwordLayout->addWidget(passwordEchoModeButton_);
  addLayout(passwordLayout);

  QHBoxLayout* def_layout = new QHBoxLayout;
  default_db_label_ = new QLabel;

  default_db_num_ = new QSpinBox;
  default_db_num_->setRange(0, INT32_MAX);
  def_layout->addWidget(default_db_label_);
  def_layout->addWidget(default_db_num_);
  addLayout(def_layout);

  // ssh

  sshWidget_ = new SSHWidget;
  QLayout* ssh_layout = sshWidget_->layout();
  ssh_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(sshWidget_);

  remote_->setChecked(true);
  selectRemoteDBPath(true);
  useAuth_->setChecked(false);
  passwordBox_->setEnabled(false);
  passwordEchoModeButton_->setEnabled(false);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::redis::ConnectionSettings* redis = static_cast<proxy::redis::ConnectionSettings*>(connection);
  if (redis) {
    core::redis::Config config = redis->GetInfo();
    bool is_remote = config.hostsocket.empty();
    if (is_remote) {
      host_widget_->setHost(config.host);
      remote_->setChecked(true);
      isSSLConnection_->setChecked(config.is_ssl);
    } else {
      QString qhostsocket;
      common::ConvertFromString(config.hostsocket, &qhostsocket);
      pathWidget_->setPath(qhostsocket);
      local_->setChecked(true);
    }

    std::string auth = config.auth;
    if (!auth.empty()) {
      useAuth_->setChecked(true);
      QString qauth;
      common::ConvertFromString(auth, &qauth);
      passwordBox_->setText(qauth);
    } else {
      useAuth_->setChecked(false);
      passwordBox_->clear();
    }
    default_db_num_->setValue(config.db_num);
    core::SSHInfo ssh_info = redis->GetSSHInfo();
    sshWidget_->setInfo(ssh_info);
  }
  ConnectionBaseWidget::syncControls(redis);
}

void ConnectionWidget::retranslateUi() {
  groupBox_->setTitle(trDBPath);
  remote_->setText(trRemote);
  isSSLConnection_->setText(trSSL);
  local_->setText(trLocal);
  useAuth_->setText(trUseAuth);
  default_db_label_->setText(trDefaultDb);
  ConnectionBaseWidget::retranslateUi();
}

void ConnectionWidget::togglePasswordEchoMode() {
  bool isPassword = passwordBox_->echoMode() == QLineEdit::Password;
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void ConnectionWidget::authStateChange(int state) {
  passwordBox_->setEnabled(state);
  passwordEchoModeButton_->setEnabled(state);
}

void ConnectionWidget::sslStateChange(int state) {
  sshWidget_->setEnabled(!state);
}

void ConnectionWidget::selectRemoteDBPath(bool checked) {
  host_widget_->setEnabled(checked);
  pathWidget_->setEnabled(!checked);
  isSSLConnection_->setEnabled(checked);
  sshWidget_->setEnabled(checked);
}

void ConnectionWidget::selectLocalDBPath(bool checked) {
  host_widget_->setEnabled(!checked);
  pathWidget_->setEnabled(checked);
  isSSLConnection_->setEnabled(!checked);
  sshWidget_->setEnabled(!checked);
}

bool ConnectionWidget::validated() const {
  if (sshWidget_->isEnabled() && sshWidget_->isSSHChecked()) {
    if (!sshWidget_->isValidSSHInfo()) {
      return false;
    }
  }

  if (!isValidCredential()) {
    return false;
  }

  bool is_remote = remote_->isChecked();
  if (is_remote) {
    if (!host_widget_->isValidHost()) {
      return false;
    }
  } else {
    if (!pathWidget_->isValidPath()) {
      return false;
    }
  }

  return ConnectionBaseWidget::validated();
}

bool ConnectionWidget::isValidCredential() const {
  if (useAuth_->isChecked()) {
    QString pass = passwordBox_->text();
    return !pass.isEmpty();
  }

  return true;
}

proxy::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(const proxy::connection_path_t& path) const {
  proxy::redis::ConnectionSettings* conn = new proxy::redis::ConnectionSettings(path);
  core::redis::Config config = conn->GetInfo();
  bool is_remote = remote_->isChecked();
  if (is_remote) {
    config.host = host_widget_->host();
    config.is_ssl = isSSLConnection_->isChecked();
  } else {
    config.hostsocket = common::ConvertToString(pathWidget_->path());
  }

  if (useAuth_->isChecked() && isValidCredential()) {
    config.auth = common::ConvertToString(passwordBox_->text());
  }
  config.db_num = default_db_num_->value();
  conn->SetInfo(config);

  core::SSHInfo info;
  if (sshWidget_->isEnabled() && sshWidget_->isSSHChecked()) {
    info = sshWidget_->info();
  }
  conn->SetSSHInfo(info);
  return conn;
}

}  // namespace redis
}  // namespace gui
}  // namespace fastonosql
