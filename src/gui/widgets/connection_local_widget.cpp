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

namespace {
const QString trDBPath = QObject::tr("Database path:");
const QString trCaption = QObject::tr("Select Database path");
const QString trFilter = QObject::tr("Database files (*.*)");
}

namespace fastonosql {
namespace gui {

ConnectionLocalWidget::ConnectionLocalWidget(bool isFolderSelectOnly, QWidget* parent)
    : ConnectionBaseWidget(parent),
      caption_(trCaption),
      filter_(trFilter),
      isFolderSelectOnly_(isFolderSelectOnly) {
  QHBoxLayout* dbNameLayout = new QHBoxLayout;
  dbPathLabel_ = new QLabel;
  dbPath_ = new QLineEdit;
  QPushButton* selectDBPathButton = new QPushButton("...");
  selectDBPathButton->setFixedSize(20, 20);
  VERIFY(connect(selectDBPathButton, &QPushButton::clicked, this,
                 &ConnectionLocalWidget::selectDBPathDialog));
  dbNameLayout->addWidget(dbPathLabel_);
  dbNameLayout->addWidget(dbPath_);
  dbNameLayout->addWidget(selectDBPathButton);
  addLayout(dbNameLayout);
}

void ConnectionLocalWidget::syncControls(core::IConnectionSettingsBase* connection) {
  core::IConnectionSettingsLocal* local = static_cast<core::IConnectionSettingsLocal*>(connection);
  if (local) {
    core::LocalConfig config = local->LocalConf();
    dbPath_->setText(common::ConvertFromString<QString>(config.dbname));
    ConnectionBaseWidget::syncControls(local);
  }
}

void ConnectionLocalWidget::retranslateUi() {
  dbPathLabel_->setText(trDBPath);
  ConnectionBaseWidget::retranslateUi();
}

void ConnectionLocalWidget::setCaption(const QString& caption) {
  caption_ = caption;
}

void ConnectionLocalWidget::setFilter(const QString& filter) {
  filter_ = filter;
}

QString ConnectionLocalWidget::DBPath() const {
  return dbPath_->text();
}

void ConnectionLocalWidget::setDBPath(const QString& path) {
  dbPath_->setText(path);
}

void ConnectionLocalWidget::selectDBPathDialog() {
  QFileDialog dialog(this, caption_, dbPath_->text(), filter_);
  dialog.setFileMode(isFolderSelectOnly_ ? QFileDialog::DirectoryOnly : QFileDialog::ExistingFile);
  int res = dialog.exec();
  if (res != QFileDialog::ExistingFile) {
    return;
  }

  QStringList files = dialog.selectedFiles();
  setDBPath(files[0]);
}

core::LocalConfig ConnectionLocalWidget::config() const {
  core::LocalConfig conf(ConnectionBaseWidget::config());
  QString db_path = dbPath_->text();
  conf.dbname = common::ConvertToString(db_path);
  return conf;
}

}  // namespace gui
}  // namespace fastonosql
