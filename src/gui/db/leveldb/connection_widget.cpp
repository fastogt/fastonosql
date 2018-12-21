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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/db/leveldb/connection_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/leveldb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace leveldb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : ConnectionLocalWidgetDirectoryPath(trDBPath, trCaption, parent) {
  create_db_if_missing_ = new QCheckBox;
  addWidget(create_db_if_missing_);

  QHBoxLayout* type_comp_layout = new QHBoxLayout;
  typeComparators_ = new QComboBox;
  for (uint32_t i = 0; i < core::leveldb::g_comparator_types.size(); ++i) {
    const char* ct = core::leveldb::g_comparator_types[i];
    typeComparators_->addItem(ct, i);
  }

  compLabel_ = new QLabel;
  type_comp_layout->addWidget(compLabel_);
  type_comp_layout->addWidget(typeComparators_);
  addLayout(type_comp_layout);

  QHBoxLayout* type_compress_layout = new QHBoxLayout;
  type_compressions_ = new QComboBox;
  for (uint32_t i = 0; i < core::leveldb::g_compression_types.size(); ++i) {
    const char* ct = core::leveldb::g_compression_types[i];
    type_compressions_->addItem(ct, i);
  }

  compression_label_ = new QLabel;
  type_compress_layout->addWidget(compression_label_);
  type_compress_layout->addWidget(type_compressions_);
  addLayout(type_compress_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::leveldb::ConnectionSettings* lev = static_cast<proxy::leveldb::ConnectionSettings*>(connection);
  if (lev) {
    core::leveldb::Config config = lev->GetInfo();
    create_db_if_missing_->setChecked(config.create_if_missing);
    typeComparators_->setCurrentIndex(config.comparator);
    type_compressions_->setCurrentIndex(config.compression);
  }
  base_class::syncControls(lev);
}

void ConnectionWidget::retranslateUi() {
  create_db_if_missing_->setText(trCreateDBIfMissing);
  compLabel_->setText(trComparator + ":");
  compression_label_->setText(trCompression + ":");
  base_class::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::leveldb::ConnectionSettings* conn = static_cast<proxy::leveldb::ConnectionSettings*>(
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(core::LEVELDB, path));
  core::leveldb::Config config = conn->GetInfo();
  config.create_if_missing = create_db_if_missing_->isChecked();
  config.comparator = static_cast<core::leveldb::ComparatorType>(typeComparators_->currentIndex());
  config.compression = static_cast<core::leveldb::CompressionType>(type_compressions_->currentIndex());
  conn->SetInfo(config);
  return conn;
}

}  // namespace leveldb
}  // namespace gui
}  // namespace fastonosql
