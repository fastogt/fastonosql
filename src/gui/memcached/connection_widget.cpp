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

#include "gui/memcached/connection_widget.h"

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "core/memcached/connection_settings.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {
namespace memcached {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {
  useSasl_ = new QCheckBox;
  VERIFY(connect(useSasl_, &QCheckBox::stateChanged, this, &ConnectionWidget::saslStateChange));
  addWidget(useSasl_);

  QHBoxLayout* userLayout = new QHBoxLayout;
  userNameLabel_ = new QLabel;
  userName_ = new QLineEdit;
  userLayout->addWidget(userNameLabel_);
  userLayout->addWidget(userName_);
  addLayout(userLayout);

  QHBoxLayout* passwordLayout = new QHBoxLayout;
  passwordLabel_ = new QLabel;
  passwordBox_ = new QLineEdit;
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this,
                 &ConnectionWidget::togglePasswordEchoMode));
  passwordLayout->addWidget(passwordLabel_);
  passwordLayout->addWidget(passwordBox_);
  passwordLayout->addWidget(passwordEchoModeButton_);
  addLayout(passwordLayout);

  // sync
  useSasl_->setChecked(false);
  userName_->setEnabled(false);
  passwordBox_->setEnabled(false);
  passwordEchoModeButton_->setEnabled(false);
}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::memcached::ConnectionSettings* memc =
      static_cast<core::memcached::ConnectionSettings*>(connection);
  if (memc) {
    core::memcached::Config config = memc->Info();
    std::string uname = config.user;
    std::string pass = config.password;
    if (!uname.empty() && !pass.empty()) {
      useSasl_->setChecked(true);
      userName_->setText(common::ConvertFromString<QString>(uname));
      passwordBox_->setText(common::ConvertFromString<QString>(pass));
    } else {
      useSasl_->setChecked(false);
      userName_->clear();
      passwordBox_->clear();
    }
  }
  ConnectionRemoteWidget::syncControls(memc);
}

void ConnectionWidget::retranslateUi() {
  useSasl_->setText(tr("Use SASL"));
  passwordLabel_->setText(tr("User Password:"));
  userNameLabel_->setText(tr("User Name:"));
  ConnectionRemoteWidget::retranslateUi();
}

void ConnectionWidget::togglePasswordEchoMode() {
  bool isPassword = passwordBox_->echoMode() == QLineEdit::Password;
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void ConnectionWidget::saslStateChange(int state) {
  userName_->setEnabled(state);
  passwordBox_->setEnabled(state);
  passwordEchoModeButton_->setEnabled(state);
}

bool ConnectionWidget::validated() const {
  if (!isValidCredential()) {
    return false;
  }

  return ConnectionRemoteWidget::validated();
}

bool ConnectionWidget::isValidCredential() const {
  if (useSasl_->isChecked()) {
    QString uname = userName_->text();
    QString pass = passwordBox_->text();
    return !uname.isEmpty() && !pass.isEmpty();
  }

  return true;
}

core::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(
    const core::connection_path_t& path) const {
  core::memcached::ConnectionSettings* conn = new core::memcached::ConnectionSettings(path);
  core::memcached::Config config(ConnectionRemoteWidget::config());
  if (useSasl_->isChecked() && isValidCredential()) {
    config.user = common::ConvertToString(userName_->text());
    config.password = common::ConvertToString(passwordBox_->text());
  }
  conn->SetInfo(config);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
