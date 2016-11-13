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

#include "gui/widgets/connection_remote_widget.h"

#include <QRegExpValidator>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "core/connection_settings/iconnection_settings_remote.h"

namespace fastonosql {
namespace gui {

ConnectionRemoteWidget::ConnectionRemoteWidget(QWidget* parent) : ConnectionBaseWidget(parent) {
  hostName_ = new QLineEdit;
  hostPort_ = new QLineEdit;
  hostPort_->setFixedWidth(80);
  QRegExp rx("\\d+");  // (0-65554)
  hostPort_->setValidator(new QRegExpValidator(rx, this));

  QHBoxLayout* hostAndPasswordLayout = new QHBoxLayout;
  hostAndPasswordLayout->addWidget(hostName_);
  hostAndPasswordLayout->addWidget(new QLabel(":"));
  hostAndPasswordLayout->addWidget(hostPort_);

  addLayout(hostAndPasswordLayout);
}

void ConnectionRemoteWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::IConnectionSettingsRemote* remote =
      static_cast<core::IConnectionSettingsRemote*>(connection);

  if (remote) {
    core::RemoteConfig config = remote->RemoteConf();
    common::net::HostAndPort host = config.host;
    hostName_->setText(common::ConvertFromString<QString>(host.host));
    hostPort_->setText(QString::number(host.port));
    ConnectionBaseWidget::syncControls(remote);
  }
}

void ConnectionRemoteWidget::retranslateUi() {
  ConnectionBaseWidget::retranslateUi();
}

core::RemoteConfig ConnectionRemoteWidget::config() const {
  core::RemoteConfig conf(ConnectionBaseWidget::config());
  conf.host = common::net::HostAndPort(common::ConvertToString(hostName_->text()),
                                       hostPort_->text().toInt());
  return conf;
}

}  // namespace gui
}  // namespace fastonosql
