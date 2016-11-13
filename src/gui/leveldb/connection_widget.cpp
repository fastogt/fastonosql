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

#include "gui/leveldb/connection_widget.h"

#include "core/leveldb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace leveldb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionLocalWidget(true, parent) {}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::leveldb::ConnectionSettings* leveldb =
      static_cast<core::leveldb::ConnectionSettings*>(connection);
  ConnectionLocalWidget::syncControls(leveldb);
}

void ConnectionWidget::retranslateUi() {
  ConnectionLocalWidget::retranslateUi();
}

core::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(
    const core::connection_path_t& path) const {
  core::leveldb::ConnectionSettings* conn = new core::leveldb::ConnectionSettings(path);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
