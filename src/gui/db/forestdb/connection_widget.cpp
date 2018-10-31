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

#include "gui/db/forestdb/connection_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <common/qt/convert2string.h>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/forestdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace forestdb {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidgetFilePath(trDBPath, trFilter, trCaption, parent) {
  QHBoxLayout* name_layout = new QHBoxLayout;
  db_name_label_ = new QLabel;
  name_layout->addWidget(db_name_label_);
  db_name_edit_ = new QLineEdit;
  name_layout->addWidget(db_name_edit_);
  addLayout(name_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::forestdb::ConnectionSettings* forestdb = static_cast<proxy::forestdb::ConnectionSettings*>(connection);
  if (forestdb) {
    core::forestdb::Config config = forestdb->GetInfo();
    QString qdb_name;
    if (common::ConvertFromString(config.db_name, &qdb_name)) {  // convert from qstring
      db_name_edit_->setText(qdb_name);
    }
  }
  ConnectionLocalWidget::syncControls(forestdb);
}

void ConnectionWidget::retranslateUi() {
  db_name_label_->setText(trDBName);
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::forestdb::ConnectionSettings* conn =
      proxy::ConnectionSettingsFactory::GetInstance().CreateFORESTDBConnection(path);
  core::forestdb::Config config = conn->GetInfo();
  config.db_name = common::ConvertToString(db_name_edit_->text());  // convert to string
  conn->SetInfo(config);
  return conn;
}

}  // namespace forestdb
}  // namespace gui
}  // namespace fastonosql
