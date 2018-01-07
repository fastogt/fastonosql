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

#include "gui/db/memcached/connection_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "proxy/db/memcached/connection_settings.h"

#include "gui/widgets/user_password_widget.h"

namespace {
const QString trUserPassword = QObject::tr("User password:");
const QString trUserName = QObject::tr("User name:");
const QString trUseSasl = QObject::tr("Use SASL");
}  // namespace

namespace fastonosql {
namespace gui {
namespace memcached {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {
  useSasl_ = new QCheckBox;
  VERIFY(connect(useSasl_, &QCheckBox::stateChanged, this, &ConnectionWidget::saslStateChange));
  addWidget(useSasl_);

  userPasswordWidget_ = new UserPasswordWidget(trUserName, trUserPassword);
  QLayout* user_layout = userPasswordWidget_->layout();
  user_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(userPasswordWidget_);

  // sync
  useSasl_->setChecked(false);
  userPasswordWidget_->setEnabled(false);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::memcached::ConnectionSettings* memc = static_cast<proxy::memcached::ConnectionSettings*>(connection);
  if (memc) {
    core::memcached::Config config = memc->GetInfo();
    std::string uname = config.user;
    std::string pass = config.password;
    bool is_valid_cred = !uname.empty() && !pass.empty();
    useSasl_->setChecked(is_valid_cred);
    QString quname;
    common::ConvertFromString(uname, &quname);
    userPasswordWidget_->setUserName(quname);

    QString qpass;
    common::ConvertFromString(pass, &qpass);
    userPasswordWidget_->setPassword(qpass);
  }
  ConnectionRemoteWidget::syncControls(memc);
}

void ConnectionWidget::retranslateUi() {
  useSasl_->setText(trUseSasl);
  ConnectionRemoteWidget::retranslateUi();
}

void ConnectionWidget::saslStateChange(int state) {
  userPasswordWidget_->setEnabled(state);
}

bool ConnectionWidget::validated() const {
  if (!isValidCredential()) {
    return false;
  }

  return ConnectionRemoteWidget::validated();
}

bool ConnectionWidget::isValidCredential() const {
  if (useSasl_->isChecked()) {
    return userPasswordWidget_->isValidCredential();
  }

  return true;
}

proxy::IConnectionSettingsRemote* ConnectionWidget::createConnectionRemoteImpl(
    const proxy::connection_path_t& path) const {
  proxy::memcached::ConnectionSettings* conn = new proxy::memcached::ConnectionSettings(path);
  core::memcached::Config config = conn->GetInfo();
  if (useSasl_->isChecked() && isValidCredential()) {
    config.user = common::ConvertToString(userPasswordWidget_->userName());
    config.password = common::ConvertToString(userPasswordWidget_->password());
  }
  conn->SetInfo(config);
  return conn;
}

}  // namespace memcached
}  // namespace gui
}  // namespace fastonosql
