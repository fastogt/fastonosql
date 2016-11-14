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

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "core/redis/connection_settings.h"

#include "gui/widgets/connection_ssh_widget.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {
namespace redis {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {
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

  ssh_widget_ = new ConnectionSSHWidget;
  QLayout* ssh_layout = ssh_widget_->layout();
  ssh_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(ssh_widget_);

  useAuth_->setChecked(false);
  passwordBox_->setEnabled(false);
  passwordEchoModeButton_->setEnabled(false);
}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::redis::ConnectionSettings* redis =
      static_cast<core::redis::ConnectionSettings*>(connection);
  if (redis) {
    core::redis::Config config = redis->Info();
    std::string auth = config.auth;
    if (!auth.empty()) {
      useAuth_->setChecked(true);
      passwordBox_->setText(common::ConvertFromString<QString>(auth));
    } else {
      useAuth_->setChecked(false);
      passwordBox_->clear();
    }
    core::SSHInfo ssh_info = redis->SSHInfo();
    ssh_widget_->setInfo(ssh_info);
  }
  ConnectionRemoteWidget::syncControls(redis);
}

void ConnectionWidget::retranslateUi() {
  useAuth_->setText(tr("Use AUTH"));
  ConnectionRemoteWidget::retranslateUi();
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

bool ConnectionWidget::validated() const {
  core::SSHInfo info = ssh_widget_->info();
  if (ssh_widget_->isSSHChecked()) {
    if (info.current_method == core::SSHInfo::PUBLICKEY && info.private_key.empty()) {
      return false;
    }
  }

  if (!isValidCredential()) {
    return false;
  }

  return ConnectionRemoteWidget::validated();
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
  core::redis::Config config(ConnectionRemoteWidget::config());
  if (useAuth_->isChecked() && isValidCredential()) {
    config.auth = common::ConvertToString(passwordBox_->text());
  }
  conn->SetInfo(config);

  core::SSHInfo info = ssh_widget_->info();
  conn->SetSSHInfo(info);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
