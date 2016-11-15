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

#include "gui/widgets/connection_local_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "core/connection_settings/iconnection_settings_local.h"

#include "gui/widgets/path_widget.h"

namespace fastonosql {
namespace gui {

ConnectionLocalWidget::ConnectionLocalWidget(bool isFolderSelectOnly,
                                             const QString& pathTitle,
                                             const QString& caption,
                                             const QString& filter,
                                             QWidget* parent)
    : ConnectionBaseWidget(parent) {
  pathWidget_ = new PathWidget(isFolderSelectOnly, pathTitle, filter, caption);
  QLayout* path_layout = pathWidget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(pathWidget_);
}

void ConnectionLocalWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::IConnectionSettingsLocal* local = static_cast<core::IConnectionSettingsLocal*>(connection);
  if (local) {
    core::LocalConfig config = local->LocalConf();
    pathWidget_->setPath(common::ConvertFromString<QString>(config.dbname));
  }
  ConnectionBaseWidget::syncControls(local);
}

void ConnectionLocalWidget::retranslateUi() {
  ConnectionBaseWidget::retranslateUi();
}

core::LocalConfig ConnectionLocalWidget::config() const {
  core::LocalConfig conf(ConnectionBaseWidget::config());
  QString db_path = pathWidget_->path();
  conf.dbname = common::ConvertToString(db_path);
  return conf;
}

}  // namespace gui
}  // namespace fastonosql
