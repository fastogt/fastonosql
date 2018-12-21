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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/db/pika/connection_widget.h"

#include <string>

#include <QCheckBox>
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

#include "proxy/connection_settings_factory.h"
#include "proxy/db/pika/connection_settings.h"

#include "gui/widgets/host_port_widget.h"
#include "gui/widgets/path_widget.h"
#include "gui/widgets/ssh_widget.h"

#include "translations/global.h"

namespace {
const QString trUnixPath = QObject::tr("Unix socket path:");
const QString trSSL = QObject::tr("SSL");
}  // namespace

namespace fastonosql {
namespace gui {
namespace pika {

ConnectionWidget::ConnectionWidget(QWidget* parent) : base_class(parent) {
  QVBoxLayout* vbox = new QVBoxLayout;

  is_ssl_connection_ = new QCheckBox;
  VERIFY(connect(is_ssl_connection_, &QCheckBox::stateChanged, this, &ConnectionWidget::sslStateChange));

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(is_ssl_connection_);

  host_widget_ = createWidget<HostPortWidget>();
  QLayout* host_layout = host_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addLayout(hbox);
  vbox->addWidget(host_widget_);
  addLayout(vbox);

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

  QHBoxLayout* def_layout = new QHBoxLayout;
  default_db_label_ = new QLabel;

  default_db_num_ = new QSpinBox;
  default_db_num_->setRange(0, INT32_MAX);
  default_db_num_->setEnabled(false);
  def_layout->addWidget(default_db_label_);
  def_layout->addWidget(default_db_num_);
  addLayout(def_layout);

  // ssh

  ssh_widget_ = createWidget<SSHWidget>();
  QLayout* ssh_layout = ssh_widget_->layout();
  ssh_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(ssh_widget_);

  use_auth_->setChecked(false);
  password_box_->setEnabled(false);
  password_echo_mode_button_->setEnabled(false);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::pika::ConnectionSettings* pika = static_cast<proxy::pika::ConnectionSettings*>(connection);
  if (pika) {
    core::pika::Config config = pika->GetInfo();
    host_widget_->setHost(config.host);
    is_ssl_connection_->setChecked(config.is_ssl);

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
    default_db_num_->setValue(config.db_num);
    core::SSHInfo ssh_info = pika->GetSSHInfo();
    ssh_widget_->setInfo(ssh_info);
  }
  base_class::syncControls(pika);
}

void ConnectionWidget::retranslateUi() {
  is_ssl_connection_->setText(trSSL);
  use_auth_->setText(trUseAuth);
  default_db_label_->setText(trDefaultDb);
  base_class::retranslateUi();
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

void ConnectionWidget::sslStateChange(int state) {
  ssh_widget_->setEnabled(!state);
}

bool ConnectionWidget::validated() const {
  if (ssh_widget_->isEnabled() && ssh_widget_->isSSHChecked()) {
    if (!ssh_widget_->isValidSSHInfo()) {
      return false;
    }
  }

  if (!isValidCredential()) {
    return false;
  }

  if (!host_widget_->isValidHost()) {
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

proxy::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(const proxy::connection_path_t& path) const {
  proxy::pika::ConnectionSettings* conn = static_cast<proxy::pika::ConnectionSettings*>(
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(core::PIKA, path));
  core::pika::Config config = conn->GetInfo();
  config.host = host_widget_->host();
  config.is_ssl = is_ssl_connection_->isChecked();

  if (use_auth_->isChecked() && isValidCredential()) {
    config.auth = common::ConvertToString(password_box_->text());
  }
  config.db_num = default_db_num_->value();
  conn->SetInfo(config);

  core::SSHInfo info;
  if (ssh_widget_->isEnabled() && ssh_widget_->isSSHChecked()) {
    info = ssh_widget_->info();
  }
  conn->SetSSHInfo(info);
  return conn;
}

}  // namespace pika
}  // namespace gui
}  // namespace fastonosql
