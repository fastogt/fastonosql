/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/db/unqlite/connection_settings.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace gui {
namespace unqlite {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(false, trDBPath, trCaption, trFilter, parent) {
  createDBIfMissing_ = new QCheckBox;
  VERIFY(connect(createDBIfMissing_, &QCheckBox::stateChanged, this, &ConnectionWidget::createDBStateChange));
  addWidget(createDBIfMissing_);
  readOnlyDB_ = new QCheckBox;
  VERIFY(connect(readOnlyDB_, &QCheckBox::stateChanged, this, &ConnectionWidget::readOnlyDBStateChange));
  addWidget(readOnlyDB_);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::unqlite::ConnectionSettings* unq = static_cast<proxy::unqlite::ConnectionSettings*>(connection);
  if (unq) {
    core::unqlite::Config config = unq->Info();
    createDBIfMissing_->setChecked(config.CreateIfMissingDB());
    readOnlyDB_->setChecked(config.ReadOnlyDB());
  }
  ConnectionLocalWidget::syncControls(unq);
}

void ConnectionWidget::retranslateUi() {
  createDBIfMissing_->setText(trCreateDBIfMissing);
  readOnlyDB_->setText(trReadOnlyDB);
  ConnectionLocalWidget::retranslateUi();
}

void ConnectionWidget::createDBStateChange(int state) {
  readOnlyDB_->setEnabled(!state);
}

void ConnectionWidget::readOnlyDBStateChange(int state) {
  createDBIfMissing_->setEnabled(!state);
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::unqlite::ConnectionSettings* conn = new proxy::unqlite::ConnectionSettings(path);
  core::unqlite::Config config = conn->Info();
  config.SetCreateIfMissingDB(createDBIfMissing_->isChecked());
  config.SetReadOnlyDB(readOnlyDB_->isChecked());
  conn->SetInfo(config);
  return conn;
}

}  // namespace unqlite
}  // namespace gui
}  // namespace fastonosql
