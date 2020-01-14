/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/db/keydb/connection_widget.h"

#include <string>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExpValidator>
#include <QSpinBox>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/keydb/connection_settings.h"

#include "gui/widgets/host_port_widget.h"
#include "gui/widgets/path_widget.h"
#include "gui/widgets/ssh_widget.h"

#include "translations/global.h"

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
#define NONE_STRING "None"
#define STACKEXCHANGE_REDIS_STRING "StackExchange.Redis"

#define PASSWORD_FIELD "password"
#define SSL_FIELD "ssl"
#endif

namespace {
const QString trUnixPath = QObject::tr("Unix socket path:");
const QString trRemote = QObject::tr("Remote");
const QString trLocal = QObject::tr("Local");
const QString trSSL = QObject::tr("SSL");
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
const QString trLoadFromConnectionString = QObject::tr("Load from connection string");
const QString trConnectionString_1S = "Connection string (%1)";
#endif
}  // namespace

namespace fastonosql {
namespace gui {
namespace keydb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : base_class(parent) {
  QVBoxLayout* vbox = new QVBoxLayout;
  group_box_ = new QGroupBox;
  remote_ = new QRadioButton;
  local_ = new QRadioButton;
  VERIFY(connect(remote_, &QRadioButton::toggled, this, &ConnectionWidget::selectRemoteDBPath));
  VERIFY(connect(local_, &QRadioButton::toggled, this, &ConnectionWidget::selectLocalDBPath));

  is_ssl_connection_ = new QCheckBox;
  VERIFY(connect(is_ssl_connection_, &QCheckBox::clicked, this, &ConnectionWidget::secureConnectionChange));

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(remote_);
  hbox->addWidget(is_ssl_connection_);

  host_widget_ = createWidget<HostPortWidget>();
  QLayout* host_layout = host_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addLayout(hbox);
  vbox->addWidget(host_widget_);

  path_widget_ = createWidget<FilePathWidget>(trUnixPath, trFilter, trCaption);
  QLayout* path_layout = path_widget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(local_);
  vbox->addWidget(path_widget_);

  group_box_->setLayout(vbox);
  addWidget(group_box_);

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
  def_layout->addWidget(default_db_label_);
  def_layout->addWidget(default_db_num_);
  addLayout(def_layout);

  // ssh
  ssh_widget_ = createWidget<SSHWidget>();
  QLayout* ssh_layout = ssh_widget_->layout();
  ssh_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(ssh_widget_);

  remote_->setChecked(true);
  selectRemoteDBPath(true);
  use_auth_->setChecked(false);
  password_box_->setEnabled(false);
  password_echo_mode_button_->setEnabled(false);

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  QHBoxLayout* hot_settings_layout = new QHBoxLayout;
  hot_settings_label_ = new QLabel;
  hot_settings_layout->addWidget(hot_settings_label_);
  hot_settings_ = new QComboBox;
  hot_settings_->addItems({NONE_STRING, STACKEXCHANGE_REDIS_STRING});
  hot_settings_layout->addWidget(hot_settings_);
  VERIFY(connect(hot_settings_, &QComboBox::currentTextChanged, this, &ConnectionWidget::updateConnectionString));
  addLayout(hot_settings_layout);
#endif
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::keydb::ConnectionSettings* redis = static_cast<proxy::keydb::ConnectionSettings*>(connection);
  if (redis) {
    core::keydb::Config config = redis->GetInfo();
    bool is_remote = config.hostsocket.empty();
    if (is_remote) {
      host_widget_->setHost(config.host);
      remote_->setChecked(true);
      is_ssl_connection_->setChecked(config.is_ssl);
    } else {
      QString qhostsocket;
      common::ConvertFromString(config.hostsocket, &qhostsocket);
      path_widget_->setPath(qhostsocket);
      local_->setChecked(true);
    }

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
    core::SSHInfo ssh_info = redis->GetSSHInfo();
    ssh_widget_->setInfo(ssh_info);
  }
  base_class::syncControls(redis);
}

void ConnectionWidget::retranslateUi() {
  group_box_->setTitle(trDBPath);
  remote_->setText(trRemote);
  is_ssl_connection_->setText(trSSL);
  local_->setText(trLocal);
  use_auth_->setText(trUseAuth);
  default_db_label_->setText(trDefaultDb);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  hot_settings_label_->setText(trLoadFromConnectionString);
#endif
  base_class::retranslateUi();
}

void ConnectionWidget::togglePasswordEchoMode() {
  bool is_password = password_box_->echoMode() == QLineEdit::Password;
  password_box_->setEchoMode(is_password ? QLineEdit::Normal : QLineEdit::Password);
  password_echo_mode_button_->setText(is_password ? translations::trHide : translations::trShow);
}

void ConnectionWidget::authStateChange(int state) {
  password_box_->setEnabled(state);
  password_echo_mode_button_->setEnabled(state);
}

void ConnectionWidget::selectRemoteDBPath(bool checked) {
  host_widget_->setEnabled(checked);
  path_widget_->setEnabled(!checked);
  is_ssl_connection_->setEnabled(checked);
  ssh_widget_->setEnabled(checked);
}

void ConnectionWidget::selectLocalDBPath(bool checked) {
  host_widget_->setEnabled(!checked);
  path_widget_->setEnabled(checked);
  is_ssl_connection_->setEnabled(!checked);
  ssh_widget_->setEnabled(!checked);
}

void ConnectionWidget::secureConnectionChange(bool checked) {
  UNUSED(checked);
#if !defined(PRO_VERSION)
  QMessageBox::information(this, translations::trProLimitations,
                           translations::trSecureConnectionAvailibleOnlyInProVersion);
  is_ssl_connection_->setChecked(false);
#endif
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
void ConnectionWidget::updateConnectionString(const QString& data) {
  if (data == NONE_STRING) {
    return;
  }

  if (data == STACKEXCHANGE_REDIS_STRING) {
    bool ok;
    QString string_text =
        QInputDialog::getText(this, STACKEXCHANGE_REDIS_STRING, trConnectionString_1S.arg(STACKEXCHANGE_REDIS_STRING),
                              QLineEdit::Normal, QString(), &ok, Qt::WindowCloseButtonHint);
    if (ok && !string_text.isEmpty()) {
      // some.redis.cache.windows.net:6380,password=rf0zcQkrVCo1oWBQDBkl2rSOvallKyFIoETwH4V0pOM=,ssl=True,abortConnect=False

      QStringList tokens = string_text.split(",");
      bool changed = false;
      for (QString setting : tokens) {
        int delem = setting.indexOf('=');
        if (delem != -1) {
          QString field = setting.mid(0, delem);
          QString value = setting.mid(delem + 1);
          if (field == PASSWORD_FIELD) {
            use_auth_->setChecked(true);
            password_box_->setText(value);
            changed = true;
          } else if (field == SSL_FIELD) {
            bool is_ssl = value == "True";
            is_ssl_connection_->setChecked(is_ssl);
            changed = true;
          }
        } else {
          common::net::HostAndPort hs;
          std::string value_str = common::ConvertToString(setting);
          if (common::ConvertFromString(value_str, &hs)) {
            remote_->setChecked(true);
            host_widget_->setHost(hs);
            changed = true;
          }
        }
      }

      if (changed) {
        ssh_widget_->setSSHChecked(false);
      }
    }
  }
}
#endif

bool ConnectionWidget::validated() const {
  if (ssh_widget_->isEnabled() && ssh_widget_->isSSHChecked()) {
    if (!ssh_widget_->isValidSSHInfo()) {
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
    if (!path_widget_->isValidPath()) {
      return false;
    }
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
  proxy::keydb::ConnectionSettings* conn = static_cast<proxy::keydb::ConnectionSettings*>(
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(core::KEYDB, path));
  core::keydb::Config config = conn->GetInfo();
  bool is_remote = remote_->isChecked();
  if (is_remote) {
    config.host = host_widget_->host();
    config.is_ssl = is_ssl_connection_->isChecked();
  } else {
    config.hostsocket = common::ConvertToString(path_widget_->path());
  }

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

}  // namespace keydb
}  // namespace gui
}  // namespace fastonosql
