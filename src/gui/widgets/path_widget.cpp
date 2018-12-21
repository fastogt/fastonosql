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

#include "gui/widgets/path_widget.h"

#include <string>

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <common/file_system/types.h>
#include <common/qt/convert2string.h>

namespace {
const QSize kPathButtonSize = QSize(26, 26);
}

namespace fastonosql {
namespace gui {

IPathWidget::IPathWidget(const QString& path_title, const QString& filter, const QString& caption, QWidget* parent)
    : base_class(parent), path_title_(path_title), filter_(filter), caption_(caption) {
  path_label_ = new QLabel;
  path_edit_ = new QLineEdit;

  QPushButton* select_path_button = new QPushButton("...");
  select_path_button->setFixedSize(kPathButtonSize);
  VERIFY(connect(select_path_button, &QPushButton::clicked, this, &IPathWidget::selectPathDialog));

  QHBoxLayout* db_name_layout = new QHBoxLayout;
  db_name_layout->addWidget(path_label_);
  db_name_layout->addWidget(path_edit_);
  db_name_layout->addWidget(select_path_button);
  setLayout(db_name_layout);
}

void IPathWidget::selectPathDialog() {
  selectPathDialogRoutine(caption_, filter_, mode());
}

void IPathWidget::selectPathDialogRoutine(const QString& caption, const QString& filter, int mode) {
  QFileDialog dialog(this, caption, path(), filter);
  dialog.setFileMode(static_cast<QFileDialog::FileMode>(mode));
  dialog.setFilter(QDir::AllDirs | QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
  int res = dialog.exec();
  if (res != QFileDialog::ExistingFile) {
    return;
  }

  QStringList files = dialog.selectedFiles();
  setPath(files[0]);
}

QString IPathWidget::path() const {
  return path_edit_->text();
}

void IPathWidget::setPath(const QString& path) {
  path_edit_->setText(path);
}

bool IPathWidget::isValidPath() const {
  const std::string path_str = common::ConvertToString(path());
  return common::file_system::is_valid_path(path_str);
}

void IPathWidget::retranslateUi() {
  path_label_->setText(path_title_);
  base_class::retranslateUi();
}

FilePathWidget::FilePathWidget(const QString& path_title,
                               const QString& filter,
                               const QString& caption,
                               QWidget* parent)
    : IPathWidget(path_title, filter, caption, parent) {}

int FilePathWidget::mode() const {
  return QFileDialog::ExistingFile;
}

DirectoryPathWidget::DirectoryPathWidget(const QString& path_title, const QString& caption, QWidget* parent)
    : IPathWidget(path_title, QString(), caption, parent) {}

int DirectoryPathWidget::mode() const {
  return QFileDialog::DirectoryOnly;
}

}  // namespace gui
}  // namespace fastonosql
