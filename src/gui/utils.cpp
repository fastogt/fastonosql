/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "gui/utils.h"

#include <QFileDialog>

namespace fastonosql {
namespace gui {

QString showSaveFileDialog(QWidget* parent, const QString& title, const QString& directory, const QString& filter) {
#if defined(OS_WIN) || defined(OS_MACOSX)
  return QFileDialog::getSaveFileName(parent, title, directory, filter);
#else
  QFileDialog dialog(parent, title, directory, filter);
  if (parent) {
    dialog.setWindowModality(Qt::WindowModal);
  }
  QRegExp filter_regex(QLatin1String("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
  QStringList filters = filter.split(QLatin1String(";;"));
  if (!filters.isEmpty()) {
    dialog.setNameFilter(filters.first());
    if (filter_regex.indexIn(filters.first()) != -1) {
      dialog.setDefaultSuffix(filter_regex.cap(1));
    }
  }
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() == QDialog::Accepted) {
    QString file_name = dialog.selectedFiles().first();
    QFileInfo info(file_name);
    if (info.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty()) {
      if (filter_regex.indexIn(dialog.selectedNameFilter()) != -1) {
        QString extension = filter_regex.cap(1);
        file_name += QLatin1String(".") + extension;
      }
    }
    return file_name;
  }

  return QString();
#endif
}

}  // namespace gui
}  // namespace fastonosql
