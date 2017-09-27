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

#include "gui/widgets/path_widget.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <QEvent>

#include <common/file_system/types.h>
#include <common/qt/convert2string.h>

namespace fastonosql {
namespace gui {

IPathWidget::IPathWidget(const QString& pathTitle, const QString& filter, const QString& caption, QWidget* parent)
    : QWidget(parent), pathTitle_(pathTitle), filter_(filter), caption_(caption) {
  QHBoxLayout* dbNameLayout = new QHBoxLayout;
  pathLabel_ = new QLabel;
  pathEdit_ = new QLineEdit;
  QPushButton* selectPathButton = new QPushButton("...");
  selectPathButton->setFixedSize(26, 26);
  VERIFY(connect(selectPathButton, &QPushButton::clicked, this, &IPathWidget::selectPathDialog));
  dbNameLayout->addWidget(pathLabel_);
  dbNameLayout->addWidget(pathEdit_);
  dbNameLayout->addWidget(selectPathButton);
  setLayout(dbNameLayout);

  retranslateUi();
}

void IPathWidget::selectPathDialog() {
  selectPathDialogRoutine(caption_, filter_, GetMode());
}

void IPathWidget::selectPathDialogRoutine(const QString& caption, const QString& filter, int mode) {
  QFileDialog dialog(this, caption, path(), filter);
  dialog.setFileMode(static_cast<QFileDialog::FileMode>(mode));
  dialog.setFilter(QDir::AllDirs | QDir::AllEntries | QDir::Hidden | QDir::System);
  int res = dialog.exec();
  if (res != QFileDialog::ExistingFile) {
    return;
  }

  QStringList files = dialog.selectedFiles();
  setPath(files[0]);
}

QString IPathWidget::path() const {
  return pathEdit_->text();
}

void IPathWidget::setPath(const QString& path) {
  pathEdit_->setText(path);
}

bool IPathWidget::isValidPath() const {
  const std::string path_str = common::ConvertToString(path());
  return common::file_system::is_valid_path(path_str);
}

void IPathWidget::retranslateUi() {
  pathLabel_->setText(pathTitle_);
}

void IPathWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

FilePathWidget::FilePathWidget(const QString& pathTitle, const QString& filter, const QString& caption, QWidget* parent)
    : IPathWidget(pathTitle, filter, caption, parent) {}

int FilePathWidget::GetMode() const {
  return QFileDialog::ExistingFile;
}

DirectoryPathWidget::DirectoryPathWidget(const QString& pathTitle, const QString& caption, QWidget* parent)
    : IPathWidget(pathTitle, QString(), caption, parent) {}

int DirectoryPathWidget::GetMode() const {
  return QFileDialog::DirectoryOnly;
}

}  // namespace gui
}  // namespace fastonosql
