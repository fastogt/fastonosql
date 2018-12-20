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

#include "gui/db/ssdb/connection_widget.h"

#include <string>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include <common/qt/convert2string.h>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/ssdb/connection_settings.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {
namespace ssdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {
  use_auth_ = new QCheckBox;
  VERIFY(connect(use_auth_, &QCheckBox::stateChanged, this, &ConnectionWidget::authStateChange));
  addWidget(use_auth_);

  QHBoxLayout* passwordLayout = new QHBoxLayout;
  password_box_ = new QLineEdit;
  password_box_->setEchoMode(QLineEdit::Password);
  password_echo_mode_button_ = new QPushButton(translations::trShow);
  VERIFY(connect(password_echo_mode_button_, &QPushButton::clicked, this, &ConnectionWidget::togglePasswordEchoMode));
  passwordLayout->addWidget(password_box_);
  passwordLayout->addWidget(password_echo_mode_button_);
  addLayout(passwordLayout);

  use_auth_->setChecked(false);
  password_box_->setEnabled(false);
  password_echo_mode_button_->setEnabled(false);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::ssdb::ConnectionSettings* ssdb = static_cast<proxy::ssdb::ConnectionSettings*>(connection);
  if (ssdb) {
    core::ssdb::Config config = ssdb->GetInfo();
    std::string auth = config.auth;
    if (!auth.empty()) {
      use_auth_->setChecked(true);
      QString qauth;
      common::ConvertFromString(auth, &qauth);
      password_box_->setText(qauth);
    } else {
      use_auth_->setChecked(false);
      password_box_->clear();
    }
  }
  base_class::syncControls(ssdb);
}

void ConnectionWidget::retranslateUi() {
  use_auth_->setText(trUseAuth);
  base_class::retranslateUi();
}

bool ConnectionWidget::validated() const {
  if (!isValidCredential()) {
    return false;
  }

  return base_class::validated();
}

bool ConnectionWidget::isValidCredential() const {
  if (use_auth_->isChecked()) {
    QString pass = password_box_->text();
    return !pass.isEmpty();
  }

  return true;
}

void ConnectionWidget::togglePasswordEchoMode() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_box_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  password_echo_mode_button_->setText(isPassword ? translations::trHide : translations::trShow);
}

void ConnectionWidget::authStateChange(int state) {
  password_box_->setEnabled(state);
  password_echo_mode_button_->setEnabled(state);
}

proxy::IConnectionSettingsRemote* ConnectionWidget::createConnectionRemoteImpl(
    const proxy::connection_path_t& path) const {
  proxy::ssdb::ConnectionSettings* conn = static_cast<proxy::ssdb::ConnectionSettings*>(
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(core::SSDB, path));
  core::ssdb::Config config = conn->GetInfo();
  if (use_auth_->isChecked() && isValidCredential()) {
    config.auth = common::ConvertToString(password_box_->text());
  }
  conn->SetInfo(config);
  return conn;
}

}  // namespace ssdb
}  // namespace gui
}  // namespace fastonosql
