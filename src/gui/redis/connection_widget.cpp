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

#include <QLayout>

#include "core/redis/connection_settings.h"

#include "gui/widgets/connection_ssh_widget.h"

namespace fastonosql {
namespace gui {
namespace redis {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {
  QLayout* main = layout();
  ssh_widget_ = new ConnectionSSHWidget;
  main->addWidget(ssh_widget_);
}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::redis::ConnectionSettings* redis =
      static_cast<core::redis::ConnectionSettings*>(connection);
  core::SSHInfo ssh_info = redis->SSHInfo();
  ssh_widget_->setInfo(ssh_info);
  ConnectionRemoteWidget::syncControls(redis);
}

void ConnectionWidget::retranslateUi() {
  ConnectionRemoteWidget::retranslateUi();
}

bool ConnectionWidget::validated() const {
  core::SSHInfo info = ssh_widget_->info();
  if (ssh_widget_->isSSHChecked()) {
    if (info.current_method == core::SSHInfo::PUBLICKEY && info.private_key.empty()) {
      return false;
    }
  }

  return ConnectionRemoteWidget::validated();
}

core::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(
    const core::connection_path_t& path) const {
  core::redis::ConnectionSettings* conn = new core::redis::ConnectionSettings(path);
  core::SSHInfo info = ssh_widget_->info();
  conn->SetSSHInfo(info);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
