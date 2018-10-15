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

#include "proxy/connection_settings_factory.h"
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

  QHBoxLayout* type_compress_layout = new QHBoxLayout;
  type_compressions_ = new QComboBox;
  for (uint32_t i = 0; i < core::rocksdb::g_compression_types.size(); ++i) {
    const char* ct = core::rocksdb::g_compression_types[i];
    type_compressions_->addItem(ct, i);
  }

  compression_label_ = new QLabel;
  type_comp_layout->addWidget(compression_label_);
  type_comp_layout->addWidget(type_compressions_);
  addLayout(type_compress_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::rocksdb::ConnectionSettings* rock = static_cast<proxy::rocksdb::ConnectionSettings*>(connection);
  if (rock) {
    core::rocksdb::Config config = rock->GetInfo();
    create_db_if_missing_->setChecked(config.create_if_missing);
    type_comparators_->setCurrentIndex(config.comparator);
    type_compressions_->setCurrentIndex(config.compression);
  }
  ConnectionLocalWidget::syncControls(rock);
}

void ConnectionWidget::retranslateUi() {
  create_db_if_missing_->setText(trCreateDBIfMissing);
  comparator_label_->setText(trComparator + ":");
  compression_label_->setText(trCompression + ":");
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::rocksdb::ConnectionSettings* conn =
      proxy::ConnectionSettingsFactory::GetInstance().CreateROCKSDBConnection(path);
  core::rocksdb::Config config = conn->GetInfo();
  config.create_if_missing = create_db_if_missing_->isChecked();
  config.comparator = static_cast<core::rocksdb::ComparatorType>(type_comparators_->currentIndex());
  config.compression = static_cast<core::rocksdb::CompressionType>(type_compressions_->currentIndex());
  conn->SetInfo(config);
  return conn;
}

}  // namespace rocksdb
}  // namespace gui
}  // namespace fastonosql
