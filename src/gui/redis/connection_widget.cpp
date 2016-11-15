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

#include "gui/redis/connection_widget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QRegExpValidator>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QEvent>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "core/redis/connection_settings.h"

#include "gui/widgets/host_port_widget.h"
#include "gui/widgets/path_widget.h"
#include "gui/widgets/ssh_widget.h"

#include "translations/global.h"

namespace {
const QString trUnixPath = QObject::tr("Unix socket path:");
const QString trCaption = QObject::tr("Select Database path");
const QString trFilter = QObject::tr("Database files (*.*)");
const QString trDBPath = QObject::tr("DB path");
const QString trRemote = QObject::tr("Remote");
const QString trLocal = QObject::tr("Local");
}

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

  hostWidget_ = new HostPortWidget;
  QLayout* host_layout = hostWidget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(remote_);
  vbox->addWidget(hostWidget_);

  pathWidget_ = new PathWidget(false, trUnixPath, trFilter, trCaption);
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
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this,
                 &ConnectionWidget::togglePasswordEchoMode));
  passwordLayout->addWidget(passwordBox_);
  passwordLayout->addWidget(passwordEchoModeButton_);
  addLayout(passwordLayout);

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

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::redis::ConnectionSettings* redis =
      static_cast<core::redis::ConnectionSettings*>(connection);
  if (redis) {
    core::redis::Config config = redis->Info();
    bool is_remote = config.hostsocket.empty();
    if (is_remote) {
      hostWidget_->setHost(config.host);
      remote_->setChecked(true);
    } else {
      pathWidget_->setPath(common::ConvertFromString<QString>(config.hostsocket));
      local_->setChecked(true);
    }

    std::string auth = config.auth;
    if (!auth.empty()) {
      useAuth_->setChecked(true);
      passwordBox_->setText(common::ConvertFromString<QString>(auth));
    } else {
      useAuth_->setChecked(false);
      passwordBox_->clear();
    }
    core::SSHInfo ssh_info = redis->SSHInfo();
    sshWidget_->setInfo(ssh_info);
  }
  ConnectionBaseWidget::syncControls(redis);
}

void ConnectionWidget::retranslateUi() {
  groupBox_->setTitle(trDBPath);
  remote_->setText(trRemote);
  local_->setText(trLocal);
  useAuth_->setText(tr("Use AUTH"));
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

void ConnectionWidget::selectRemoteDBPath(bool checked) {
  hostWidget_->setEnabled(checked);
  pathWidget_->setEnabled(!checked);
}

void ConnectionWidget::selectLocalDBPath(bool checked) {
  hostWidget_->setEnabled(!checked);
  pathWidget_->setEnabled(checked);
}

bool ConnectionWidget::validated() const {
  if (sshWidget_->isSSHChecked()) {
    if (!sshWidget_->isValidSSHInfo()) {
      return false;
    }
  }

  if (!isValidCredential()) {
    return false;
  }

  bool is_remote = remote_->isChecked();
  if (is_remote) {
    if (!hostWidget_->isValidHost()) {
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

core::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(
    const core::connection_path_t& path) const {
  core::redis::ConnectionSettings* conn = new core::redis::ConnectionSettings(path);
  core::RemoteConfig rconf(ConnectionBaseWidget::config());
  core::redis::Config config(rconf);
  bool is_remote = remote_->isChecked();
  if (is_remote) {
    config.host = hostWidget_->host();
  } else {
    config.hostsocket = common::ConvertToString(pathWidget_->path());
  }

  if (useAuth_->isChecked() && isValidCredential()) {
    config.auth = common::ConvertToString(passwordBox_->text());
  }
  conn->SetInfo(config);

  core::SSHInfo info;
  if (sshWidget_->isSSHChecked()) {
    info = sshWidget_->info();
  }
  conn->SetSSHInfo(info);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
