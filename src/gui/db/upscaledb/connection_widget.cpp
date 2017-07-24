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

#include "gui/db/upscaledb/connection_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include "proxy/db/upscaledb/connection_settings.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace {
const QString trDefaultDb = QObject::tr("Default database:");
}

namespace fastonosql {
namespace gui {
namespace upscaledb {

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : ConnectionLocalWidget(false, trDBPath, trCaption, trFilter, parent) {
  createDBIfMissing_ = new QCheckBox;
  addWidget(createDBIfMissing_);

  QHBoxLayout* def_layout = new QHBoxLayout;
  defaultDBLabel_ = new QLabel;

  defaultDBNum_ = new QSpinBox;
  defaultDBNum_->setRange(1, INT16_MAX);
  def_layout->addWidget(defaultDBLabel_);
  def_layout->addWidget(defaultDBNum_);
  addLayout(def_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::upscaledb::ConnectionSettings* ups =
      static_cast<proxy::upscaledb::ConnectionSettings*>(connection);
  if (ups) {
    core::upscaledb::Config config = ups->Info();
    createDBIfMissing_->setChecked(config.create_if_missing);
    defaultDBNum_->setValue(config.dbnum);
  }
  ConnectionLocalWidget::syncControls(ups);
}

void ConnectionWidget::retranslateUi() {
  createDBIfMissing_->setText(trCreateDBIfMissing);
  defaultDBLabel_->setText(trDefaultDb);
  ConnectionLocalWidget::retranslateUi();
}

proxy::IConnectionSettingsLocal* ConnectionWidget::createConnectionLocalImpl(
    const proxy::connection_path_t& path) const {
  proxy::upscaledb::ConnectionSettings* conn = new proxy::upscaledb::ConnectionSettings(path);
  core::upscaledb::Config config = conn->Info();
  config.create_if_missing = createDBIfMissing_->isChecked();
  config.dbnum = defaultDBNum_->value();
  conn->SetInfo(config);
  return conn;
}

}  // namespace upscaledb
}  // namespace gui
}  // namespace fastonosql
