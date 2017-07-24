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

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>

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

void ConnectionLocalWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::IConnectionSettingsLocal* local =
      static_cast<proxy::IConnectionSettingsLocal*>(connection);
  if (local) {
    QString db_path;
    common::ConvertFromString(local->DBPath(), &db_path);
    pathWidget_->setPath(db_path);
  }
  ConnectionBaseWidget::syncControls(local);
}

void ConnectionLocalWidget::retranslateUi() {
  ConnectionBaseWidget::retranslateUi();
}

bool ConnectionLocalWidget::validated() const {
  if (!pathWidget_->isValidPath()) {
    return false;
  }

  return ConnectionBaseWidget::validated();
}

proxy::IConnectionSettingsBase* ConnectionLocalWidget::createConnectionImpl(
    const proxy::connection_path_t& path) const {
  proxy::IConnectionSettingsLocal* local = createConnectionLocalImpl(path);
  local->SetDBPath(common::ConvertToString(pathWidget_->path()));
  return local;
}

}  // namespace gui
}  // namespace fastonosql
