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

#include "gui/db/forestdb/connection_widget.h"

#include <QCheckBox>

#include "proxy/db/forestdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace forestdb {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(true, trDBPath, trCaption, trFilter, parent) {
  readOnlyDB_ = new QCheckBox;
  addWidget(readOnlyDB_);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::forestdb::ConnectionSettings* forestdb = static_cast<proxy::forestdb::ConnectionSettings*>(connection);
  if (forestdb) {
    core::forestdb::Config config = forestdb->Info();
    readOnlyDB_->setChecked(config.ReadOnlyDB());
  }
  ConnectionLocalWidget::syncControls(forestdb);
}

void ConnectionWidget::retranslateUi() {
  readOnlyDB_->setText(trReadOnlyDB);
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::forestdb::ConnectionSettings* conn = new proxy::forestdb::ConnectionSettings(path);
  core::forestdb::Config config = conn->Info();
  config.SetReadOnlyDB(readOnlyDB_->isChecked());
  conn->SetInfo(config);
  return conn;
}

}  // namespace forestdb
}  // namespace gui
}  // namespace fastonosql
