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

#include "gui/db/upscaledb/connection_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include "proxy/connection_settings_factory.h"
#include "proxy/db/upscaledb/connection_settings.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace gui {
namespace upscaledb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : base_class(trDBPath, trFilter, trCaption, parent) {
  create_db_if_missing_ = new QCheckBox;
  addWidget(create_db_if_missing_);

  QHBoxLayout* def_layout = new QHBoxLayout;
  default_db_label_ = new QLabel;

  default_db_num_ = new QSpinBox;
  default_db_num_->setRange(1, INT16_MAX);
  def_layout->addWidget(default_db_label_);
  def_layout->addWidget(default_db_num_);
  addLayout(def_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::upscaledb::ConnectionSettings* ups = static_cast<proxy::upscaledb::ConnectionSettings*>(connection);
  if (ups) {
    core::upscaledb::Config config = ups->GetInfo();
    create_db_if_missing_->setChecked(config.create_if_missing);
    default_db_num_->setValue(config.dbnum);
  }
  base_class::syncControls(ups);
}

void ConnectionWidget::retranslateUi() {
  create_db_if_missing_->setText(trCreateDBIfMissing);
  default_db_label_->setText(trDefaultDb);
  base_class::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::upscaledb::ConnectionSettings* conn = static_cast<proxy::upscaledb::ConnectionSettings*>(
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(core::UPSCALEDB, path));
  core::upscaledb::Config config = conn->GetInfo();
  config.create_if_missing = create_db_if_missing_->isChecked();
  config.dbnum = default_db_num_->value();
  conn->SetInfo(config);
  return conn;
}

}  // namespace upscaledb
}  // namespace gui
}  // namespace fastonosql
