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

#include <QDialog>

#include <fastonosql/core/connection_types.h>  // for ConnectionTypes
#include <fastonosql/core/db_key.h>            // for NDbKValue, NValue

class QLineEdit;
class QComboBox;
class QGroupBox;
class QTableView;
class QLabel;

namespace fastonosql {
namespace gui {

class FastoEditor;
class HashTypeWidget;
class StreamTypeWidget;
class ListTypeWidget;

class DbKeyDialog : public QDialog {
  Q_OBJECT
 public:
  static const QSize min_dialog_size;

  explicit DbKeyDialog(const QString& title,
                       core::ConnectionTypes type,
                       const core::NDbKValue& key,
                       bool is_edit,
                       QWidget* parent = Q_NULLPTR);
  core::NDbKValue key() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void typeChanged(int index);

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void syncControls(common::Value* item);
  bool validateAndApply();
  void retranslateUi();

  common::Value* item() const;
  QGroupBox* general_box_;
  QLabel* type_label_;
  QLabel* key_label_;
  QLineEdit* key_edit_;
  QComboBox* types_combo_box_;
  QLabel* value_label_;
  QLineEdit* value_edit_;
  FastoEditor* json_value_edit_;
  QComboBox* bool_value_edit_;
  ListTypeWidget* value_list_edit_;
  HashTypeWidget* value_table_edit_;
  StreamTypeWidget* stream_table_edit_;

  core::NDbKValue key_;
};

}  // namespace gui
}  // namespace fastonosql
