/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/widgets/connection_local_widget.h"

#include <QHBoxLayout>

#include <common/qt/convert2string.h>

#include "gui/widgets/path_widget.h"

namespace fastonosql {
namespace gui {

ConnectionLocalWidget::ConnectionLocalWidget(IPathWidget* path_widget, QWidget* parent)
    : base_class(parent), path_widget_(path_widget) {
  CHECK(path_widget);

  QLayout* path_layout = path_widget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  addWidget(path_widget_);
}

void ConnectionLocalWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::IConnectionSettingsLocal* local = static_cast<proxy::IConnectionSettingsLocal*>(connection);
  if (local) {
    QString db_path;
    common::ConvertFromString(local->GetDBPath(), &db_path);
    path_widget_->setPath(db_path);
  }
  base_class::syncControls(local);
}

bool ConnectionLocalWidget::validated() const {
  if (!path_widget_->isValidPath()) {
    return false;
  }

  return base_class::validated();
}

proxy::IConnectionSettingsBase* ConnectionLocalWidget::createConnectionImpl(
    const proxy::connection_path_t& path) const {
  proxy::IConnectionSettingsLocal* local = createConnectionLocalImpl(path);
  local->SetDBPath(common::ConvertToString(path_widget_->path()));
  return local;
}

ConnectionLocalWidgetDirectoryPath::ConnectionLocalWidgetDirectoryPath(const QString& path_title,
                                                                       const QString& caption,
                                                                       QWidget* parent)
    : ConnectionLocalWidget(createWidget<DirectoryPathWidget>(path_title, caption), parent) {}

ConnectionLocalWidgetFilePath::ConnectionLocalWidgetFilePath(const QString& path_title,
                                                             const QString& filter,
                                                             const QString& caption,
                                                             QWidget* parent)
    : ConnectionLocalWidget(createWidget<FilePathWidget>(path_title, filter, caption), parent) {}

}  // namespace gui
}  // namespace fastonosql
