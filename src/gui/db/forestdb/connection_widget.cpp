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

#include "gui/db/forestdb/connection_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <common/qt/convert2string.h>

#include "proxy/db/forestdb/connection_settings.h"

namespace {
const QString trDBName = QObject::tr("Database name:");
}

namespace fastonosql {
namespace gui {
namespace forestdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionLocalWidgetDirectoryPath(trDBPath, trFilter, parent) {
  QHBoxLayout* name_layout = new QHBoxLayout;
  nameLabel_ = new QLabel;
  name_layout->addWidget(nameLabel_);
  nameEdit_ = new QLineEdit;
  name_layout->addWidget(nameEdit_);
  addLayout(name_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::forestdb::ConnectionSettings* forestdb = static_cast<proxy::forestdb::ConnectionSettings*>(connection);
  if (forestdb) {
    core::forestdb::Config config = forestdb->GetInfo();
    QString qdb_name;
    if (common::ConvertFromString(config.db_name, &qdb_name)) {
      nameEdit_->setText(qdb_name);
    }
  }
  ConnectionLocalWidget::syncControls(forestdb);
}

void ConnectionWidget::retranslateUi() {
  nameLabel_->setText(trDBName);
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::forestdb::ConnectionSettings* conn = new proxy::forestdb::ConnectionSettings(path);
  core::forestdb::Config config = conn->GetInfo();
  config.db_name = common::ConvertToString(nameEdit_->text());
  conn->SetInfo(config);
  return conn;
}

}  // namespace forestdb
}  // namespace gui
}  // namespace fastonosql
