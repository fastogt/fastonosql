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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "gui/widgets/connection_base_widget.h"

class QGroupBox;
class QRadioButton;

namespace fastonosql {
namespace gui {
class IPathWidget;
namespace lmdb {

class ConnectionWidget : public ConnectionBaseWidget {
  Q_OBJECT
 public:
  typedef ConnectionBaseWidget base_class;
  explicit ConnectionWidget(QWidget* parent = Q_NULLPTR);

  void syncControls(proxy::IConnectionSettingsBase* connection) override;
  void retranslateUi() override;
  bool validated() const override;

 private Q_SLOTS:
  void selectFilePathDB(bool checked);
  void selectDirectoryPathDB(bool checked);

 private:
  proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const override;

  QCheckBox* read_only_db_;
  QGroupBox* group_box_;
  QRadioButton* file_path_selection_;
  QRadioButton* directory_path_selection_;

  IPathWidget* file_path_widget_;
  IPathWidget* directory_path_widget_;

  QLabel* db_name_label_;
  QLineEdit* db_name_edit_;

  QLabel* max_dbs_count_label_;
  QSpinBox* max_dbs_count_edit_;
};

}  // namespace lmdb
}  // namespace gui
}  // namespace fastonosql
