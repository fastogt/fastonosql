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

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "gui/widgets/host_port_widget.h"

namespace fastonosql {
namespace gui {

ConnectionRemoteWidget::ConnectionRemoteWidget(QWidget* parent) : ConnectionBaseWidget(parent), hostWidget_(nullptr) {
  hostWidget_ = new HostPortWidget;
  QLayout* host_layout = hostWidget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(hostWidget_);
}

void ConnectionRemoteWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::IConnectionSettingsRemote* remote = static_cast<proxy::IConnectionSettingsRemote*>(connection);

  if (remote) {
    common::net::HostAndPort host = remote->Host();
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

proxy::IConnectionSettingsBase* ConnectionRemoteWidget::createConnectionImpl(
    const proxy::connection_path_t& path) const {
  proxy::IConnectionSettingsRemote* remote = createConnectionRemoteImpl(path);
  remote->SetHost(hostWidget_->host());
  return remote;
}

}  // namespace gui
}  // namespace fastonosql
