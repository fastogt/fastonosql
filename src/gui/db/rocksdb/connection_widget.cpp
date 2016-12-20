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

#include "gui/db/rocksdb/connection_widget.h"

#include <QCheckBox>

#include "proxy/db/rocksdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace rocksdb {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(true, trDBPath, trCaption, trFilter, parent) {
  createDBIfMissing_ = new QCheckBox;
  addWidget(createDBIfMissing_);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::rocksdb::ConnectionSettings* rock =
      static_cast<proxy::rocksdb::ConnectionSettings*>(connection);
  if (rock) {
    core::rocksdb::Config config = rock->Info();
    createDBIfMissing_->setChecked(config.options.create_if_missing);
  }
  ConnectionLocalWidget::syncControls(rock);
}

void ConnectionWidget::retranslateUi() {
  createDBIfMissing_->setText(trCreateDBIfMissing);
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::rocksdb::ConnectionSettings* conn = new proxy::rocksdb::ConnectionSettings(path);
  core::rocksdb::Config config = conn->Info();
  config.options.create_if_missing = createDBIfMissing_->isChecked();
  conn->SetInfo(config);
  return conn;
}

}  // namespace rocksdb
}  // namespace gui
}  // namespace fastonosql
