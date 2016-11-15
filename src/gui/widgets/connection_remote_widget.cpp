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

#include "gui/widgets/host_port_widget.h"

namespace fastonosql {
namespace gui {

ConnectionRemoteWidget::ConnectionRemoteWidget(QWidget* parent)
    : ConnectionBaseWidget(parent), hostWidget_(nullptr) {
  hostWidget_ = new HostPortWidget;
  QLayout* host_layout = hostWidget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(hostWidget_);
}

void ConnectionRemoteWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::IConnectionSettingsRemote* remote =
      static_cast<core::IConnectionSettingsRemote*>(connection);

  if (remote) {
    core::RemoteConfig config = remote->RemoteConf();
    common::net::HostAndPort host = config.host;
    hostWidget_->setHost(host);
  }
  ConnectionBaseWidget::syncControls(remote);
}

void ConnectionRemoteWidget::retranslateUi() {
  ConnectionBaseWidget::retranslateUi();
}

bool ConnectionRemoteWidget::validated() const {
  if (!hostWidget_->isValidHost()) {
    return false;
  }

  return ConnectionBaseWidget::validated();
}

core::RemoteConfig ConnectionRemoteWidget::config() const {
  core::RemoteConfig conf(ConnectionBaseWidget::config());
  conf.host = hostWidget_->host();
  return conf;
}

}  // namespace gui
}  // namespace fastonosql
