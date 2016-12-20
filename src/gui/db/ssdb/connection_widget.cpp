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

#include "gui/db/ssdb/connection_widget.h"

#include "proxy/db/ssdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace ssdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::IConnectionSettingsRemote* remote =
      static_cast<proxy::IConnectionSettingsRemote*>(connection);
  ConnectionRemoteWidget::syncControls(remote);
}

void ConnectionWidget::retranslateUi() {
  ConnectionRemoteWidget::retranslateUi();
}

proxy::IConnectionSettingsRemote* ConnectionWidget::createConnectionRemoteImpl(
    const proxy::connection_path_t& path) const {
  proxy::ssdb::ConnectionSettings* conn = new proxy::ssdb::ConnectionSettings(path);
  return conn;
}

}  // namespace ssdb
}  // namespace gui
}  // namespace fastonosql
