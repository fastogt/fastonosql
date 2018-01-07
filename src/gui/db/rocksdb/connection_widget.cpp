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

#include "gui/db/rocksdb/connection_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include "proxy/db/rocksdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace rocksdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionLocalWidgetDirectoryPath(trDBPath, trCaption, parent) {
  create_db_if_missing_ = new QCheckBox;
  addWidget(create_db_if_missing_);

  QHBoxLayout* type_comp_layout = new QHBoxLayout;
  type_comparators_ = new QComboBox;
  for (uint32_t i = 0; i < core::rocksdb::g_comparator_types.size(); ++i) {
    const char* ct = core::rocksdb::g_comparator_types[i];
    type_comparators_->addItem(ct, i);
  }

  comparator_label_ = new QLabel;
  type_comp_layout->addWidget(comparator_label_);
  type_comp_layout->addWidget(type_comparators_);
  addLayout(type_comp_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::rocksdb::ConnectionSettings* rock = static_cast<proxy::rocksdb::ConnectionSettings*>(connection);
  if (rock) {
    core::rocksdb::Config config = rock->GetInfo();
    create_db_if_missing_->setChecked(config.create_if_missing);
    type_comparators_->setCurrentIndex(config.comparator);
  }
  ConnectionLocalWidget::syncControls(rock);
}

void ConnectionWidget::retranslateUi() {
  create_db_if_missing_->setText(trCreateDBIfMissing);
  comparator_label_->setText(trComparator + ":");
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::rocksdb::ConnectionSettings* conn = new proxy::rocksdb::ConnectionSettings(path);
  core::rocksdb::Config config = conn->GetInfo();
  config.create_if_missing = create_db_if_missing_->isChecked();
  config.comparator = static_cast<core::rocksdb::ComparatorType>(type_comparators_->currentIndex());
  conn->SetInfo(config);
  return conn;
}

}  // namespace rocksdb
}  // namespace gui
}  // namespace fastonosql
