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

#include "gui/widgets/path_widget.h"

#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExpValidator>

#include <common/convert2string.h>
#include <common/file_system.h>
#include <common/qt/convert2string.h>

namespace fastonosql {
namespace gui {

PathWidget::PathWidget(bool isFolderSelectOnly,
                       const QString& pathTitle,
                       const QString& filter,
                       const QString& caption,
                       QWidget* parent)
    : QWidget(parent),
      pathTitle_(pathTitle),
      filter_(filter),
      caption_(caption),
      isFolderSelectOnly_(isFolderSelectOnly) {
  QHBoxLayout* dbNameLayout = new QHBoxLayout;
  pathLabel_ = new QLabel;
  pathEdit_ = new QLineEdit;
  QPushButton* selectPathButton = new QPushButton("...");
  selectPathButton->setFixedSize(26, 26);
  VERIFY(connect(selectPathButton, &QPushButton::clicked, this, &PathWidget::selectPathDialog));
  dbNameLayout->addWidget(pathLabel_);
  dbNameLayout->addWidget(pathEdit_);
  dbNameLayout->addWidget(selectPathButton);
  setLayout(dbNameLayout);

  retranslateUi();
}

void PathWidget::selectPathDialog() {
  QFileDialog dialog(this, caption_, pathEdit_->text(), filter_);
  dialog.setFileMode(isFolderSelectOnly_ ? QFileDialog::DirectoryOnly : QFileDialog::ExistingFile);
  dialog.setFilter(QDir::AllDirs | QDir::AllEntries | QDir::Hidden | QDir::System);
  int res = dialog.exec();
  if (res != QFileDialog::ExistingFile) {
    return;
  }

  QStringList files = dialog.selectedFiles();
  setPath(files[0]);
}

QString PathWidget::path() const {
  return pathEdit_->text();
}

void PathWidget::setPath(const QString& path) {
  pathEdit_->setText(path);
}

bool PathWidget::isValidPath() const {
  std::string path_str = common::ConvertToString(path());
  return common::file_system::is_valid_path(path_str);
}

void PathWidget::retranslateUi() {
  pathLabel_->setText(pathTitle_);
}

void PathWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

}  // namespace gui
}  // namespace fastonosql
