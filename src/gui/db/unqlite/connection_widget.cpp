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

#include "gui/db/unqlite/connection_widget.h"

#include <QCheckBox>

#include "core/db/unqlite/connection_settings.h"

#include "core/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace gui {
namespace unqlite {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(false, trDBPath, trCaption, trFilter, parent) {
  createDBIfMissing_ = new QCheckBox;
  addWidget(createDBIfMissing_);
}

void ConnectionWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::unqlite::ConnectionSettings* unq =
      static_cast<core::unqlite::ConnectionSettings*>(connection);
  if (unq) {
    core::unqlite::Config config = unq->Info();
    createDBIfMissing_->setChecked(config.create_if_missing);
  }
  ConnectionLocalWidget::syncControls(unq);
}

void ConnectionWidget::retranslateUi() {
  createDBIfMissing_->setText(trCreateDBIfMissing);
  ConnectionLocalWidget::retranslateUi();
}

core::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const core::connection_path_t& path) const {
  core::unqlite::ConnectionSettings* conn = new core::unqlite::ConnectionSettings(path);
  core::unqlite::Config config = conn->Info();
  config.create_if_missing = createDBIfMissing_->isChecked();
  conn->SetInfo(config);
  return conn;
}

}  // namespace unqlite
}  // namespace gui
}  // namespace fastonosql
