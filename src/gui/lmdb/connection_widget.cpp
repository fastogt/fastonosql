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

#include "gui/lmdb/connection_widget.h"

#include <QCheckBox>

#include "core/lmdb/connection_settings.h"

#include "core/connection_settings/iconnection_settings_local.h"

namespace {
const QString trCreateDBIfMissing = QObject::tr("Create database");
}

namespace fastonosql {
namespace gui {
namespace lmdb {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(true, trDBPath, trCaption, trFilter, parent) {
}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::lmdb::ConnectionSettings* lmdb = static_cast<core::lmdb::ConnectionSettings*>(connection);
  ConnectionLocalWidget::syncControls(lmdb);
}

void ConnectionWidget::retranslateUi() {
  ConnectionLocalWidget::retranslateUi();
}

core::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(
    const core::connection_path_t& path) const {
  core::lmdb::ConnectionSettings* conn = new core::lmdb::ConnectionSettings(path);
  core::lmdb::Config config(ConnectionLocalWidget::config());
  conn->SetInfo(config);
  return conn;
}
}
}  // namespace gui
}  // namespace fastonosql
