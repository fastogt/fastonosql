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

#include "gui/widgets/connection_remote_widget.h"

#include <QHBoxLayout>

#include "gui/widgets/host_port_widget.h"

namespace fastonosql {
namespace gui {

ConnectionRemoteWidget::ConnectionRemoteWidget(QWidget* parent) : base_class(parent), host_widget_(nullptr) {
  host_widget_ = createWidget<HostPortWidget>();
  QLayout* host_layout = host_widget_->layout();
  host_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(host_widget_);
}

void ConnectionRemoteWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::IConnectionSettingsRemote* remote = static_cast<proxy::IConnectionSettingsRemote*>(connection);

  if (remote) {
    common::net::HostAndPort host = remote->GetHost();
    host_widget_->setHost(host);
  }
  base_class::syncControls(remote);
}

bool ConnectionRemoteWidget::validated() const {
  if (!host_widget_->isValidHost()) {
    return false;
  }

  return base_class::validated();
}

proxy::IConnectionSettingsBase* ConnectionRemoteWidget::createConnectionImpl(
    const proxy::connection_path_t& path) const {
  proxy::IConnectionSettingsRemote* remote = createConnectionRemoteImpl(path);
  remote->SetHost(host_widget_->host());
  return remote;
}

}  // namespace gui
}  // namespace fastonosql
