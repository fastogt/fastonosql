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

#include "gui/db/lmdb/connection_widget.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>

#include <common/qt/convert2string.h>

#include "gui/widgets/path_widget.h"
#include "translations/global.h"

#include "proxy/db/lmdb/connection_settings.h"

namespace fastonosql {
namespace gui {
namespace lmdb {

ConnectionWidget::ConnectionWidget(QWidget* parent) : base_class(parent) {
  QVBoxLayout* vbox = new QVBoxLayout;
  group_box_ = new QGroupBox;
  file_path_selection_ = new QRadioButton;
  directory_path_selection_ = new QRadioButton;
  VERIFY(connect(file_path_selection_, &QRadioButton::toggled, this, &ConnectionWidget::selectFilePathDB));
  VERIFY(connect(directory_path_selection_, &QRadioButton::toggled, this, &ConnectionWidget::selectDirectoryPathDB));

  file_path_widget_ = new FilePathWidget(trDBPath, trFilter, trCaption);
  QLayout* path_layout = file_path_widget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(file_path_selection_);
  vbox->addWidget(file_path_widget_);

  directory_path_widget_ = new DirectoryPathWidget(trDBPath, trCaption);
  path_layout = directory_path_widget_->layout();
  path_layout->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(directory_path_selection_);
  vbox->addWidget(directory_path_widget_);

  group_box_->setLayout(vbox);
  addWidget(group_box_);

  read_only_db_ = new QCheckBox;
  addWidget(read_only_db_);

  QHBoxLayout* name_layout = new QHBoxLayout;
  db_name_label_ = new QLabel;
  name_layout->addWidget(db_name_label_);
  db_name_edit_ = new QLineEdit;
  name_layout->addWidget(db_name_edit_);
  addLayout(name_layout);
}

void ConnectionWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  proxy::lmdb::ConnectionSettings* lmdb = static_cast<proxy::lmdb::ConnectionSettings*>(connection);
  if (lmdb) {
    core::lmdb::Config config = lmdb->GetInfo();
    read_only_db_->setChecked(config.ReadOnlyDB());
    bool is_file_path = config.IsSingleFileDB();
    QString db_path;
    common::ConvertFromString(lmdb->GetDBPath(), &db_path);
    if (is_file_path) {
      file_path_widget_->setPath(db_path);
      file_path_selection_->setChecked(true);
    } else {
      directory_path_widget_->setPath(db_path);
      directory_path_selection_->setChecked(true);
    }

    QString qdb_name;
    if (common::ConvertFromString(config.db_name, &qdb_name)) {
      db_name_edit_->setText(qdb_name);
    }
  }
  base_class::syncControls(lmdb);
}

void ConnectionWidget::retranslateUi() {
  group_box_->setTitle(trDBPath);
  file_path_selection_->setText(translations::trFile);
  directory_path_selection_->setText(translations::trDirectory);
  read_only_db_->setText(trReadOnlyDB);
  db_name_label_->setText(trDBName);
  base_class::retranslateUi();
}

bool ConnectionWidget::validated() const {
  bool is_file_path = file_path_selection_->isChecked();
  if (is_file_path) {
    if (!file_path_widget_->isValidPath()) {
      return false;
    }
  } else {
    if (!directory_path_widget_->isValidPath()) {
      return false;
    }
  }

  return ConnectionBaseWidget::validated();
}

void ConnectionWidget::selectFilePathDB(bool checked) {
  file_path_widget_->setEnabled(checked);
  directory_path_widget_->setEnabled(!checked);
}

void ConnectionWidget::selectDirectoryPathDB(bool checked) {
  directory_path_widget_->setEnabled(checked);
  file_path_widget_->setEnabled(!checked);
}

proxy::IConnectionSettingsBase* ConnectionWidget::createConnectionImpl(const proxy::connection_path_t& path) const {
  proxy::lmdb::ConnectionSettings* conn = new proxy::lmdb::ConnectionSettings(path);
  core::lmdb::Config config = conn->GetInfo();
  config.SetReadOnlyDB(read_only_db_->isChecked());
  bool is_file_path = file_path_selection_->isChecked();
  if (is_file_path) {
    config.db_path = common::ConvertToString(file_path_widget_->path());
  } else {
    config.db_path = common::ConvertToString(directory_path_widget_->path());
  }
  config.db_name = common::ConvertToString(db_name_edit_->text());
  config.SetSingleFileDB(is_file_path);
  conn->SetInfo(config);
  return conn;
}

}  // namespace lmdb
}  // namespace gui
}  // namespace fastonosql
