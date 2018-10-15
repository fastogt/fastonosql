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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/db/unqlite/connection_widget.h"

#include <QCheckBox>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/unqlite/connection_settings.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace gui {
namespace unqlite {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidgetFilePath(trDBPath, trFilter, trCaption, parent) {
  create_db_if_missing_ = new QCheckBox;
  VERIFY(connect(create_db_if_missing_, &QCheckBox::stateChanged, this, &ConnectionWidget::createDBStateChange));
  addWidget(create_db_if_missing_);
  read_only_db_ = new QCheckBox;
  VERIFY(connect(read_only_db_, &QCheckBox::stateChanged, this, &ConnectionWidget::readOnlyDBStateChange));
  addWidget(read_only_db_);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::unqlite::ConnectionSettings* unq = static_cast<proxy::unqlite::ConnectionSettings*>(connection);
  if (unq) {
    core::unqlite::Config config = unq->GetInfo();
    create_db_if_missing_->setChecked(config.CreateIfMissingDB());
    read_only_db_->setChecked(config.ReadOnlyDB());
  }
  ConnectionLocalWidget::syncControls(unq);
}

void ConnectionWidget::retranslateUi() {
  create_db_if_missing_->setText(trCreateDBIfMissing);
  read_only_db_->setText(trReadOnlyDB);
  ConnectionLocalWidget::retranslateUi();
}

void ConnectionWidget::createDBStateChange(int state) {
  read_only_db_->setEnabled(!state);
}

void ConnectionWidget::readOnlyDBStateChange(int state) {
  create_db_if_missing_->setEnabled(!state);
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::unqlite::ConnectionSettings* conn =
      proxy::ConnectionSettingsFactory::GetInstance().CreateUNQLITEConnection(path);
  core::unqlite::Config config = conn->GetInfo();
  config.SetCreateIfMissingDB(create_db_if_missing_->isChecked());
  config.SetReadOnlyDB(read_only_db_->isChecked());
  conn->SetInfo(config);
  return conn;
}

}  // namespace unqlite
}  // namespace gui
}  // namespace fastonosql
