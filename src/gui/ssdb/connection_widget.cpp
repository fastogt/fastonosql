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

#include "gui/ssdb/connection_widget.h"

#include "core/ssdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace ssdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionRemoteWidget(parent) {}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::IConnectionSettingsRemote* remote =
      static_cast<core::IConnectionSettingsRemote*>(connection);
  ConnectionRemoteWidget::syncControls(remote);
}

void ConnectionWidget::retranslateUi() {
  ConnectionRemoteWidget::retranslateUi();
}

core::IConnectionSettingsRemote* ConnectionWidget::createConnectionRemoteImpl(
    const core::connection_path_t& path) const {
  core::ssdb::ConnectionSettings* conn = new core::ssdb::ConnectionSettings(path);
  return conn;
}

}  // namespace ssdb
}  // namespace gui
}  // namespace fastonosql
