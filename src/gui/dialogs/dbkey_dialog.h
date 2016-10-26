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

#pragma once

#include <QDialog>

#include "core/connection_types.h"  // for connectionTypes
#include "core/db_key.h"            // for NDbKValue, NValue

class QLineEdit;
class QComboBox;
class QListWidget;
class QTableWidget;
class QGroupBox;

namespace fastonosql {
namespace gui {
class DbKeyDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_width = 320, min_height = 200 };

  explicit DbKeyDialog(const QString& title,
                       core::connectionTypes type,
                       core::NDbKValue key = core::NDbKValue(),
                       QWidget* parent = 0);
  core::NDbKValue Key() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void TypeChanged(int index);
  void AddItem();
  void RemoveItem();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void SyncControls(common::Value* item);
  bool ValidateAndApply();
  void RetranslateUi();

  common::Value* Item() const;
  const core::connectionTypes type_;
  QGroupBox* generalBox_;
  QLineEdit* keyEdit_;
  QComboBox* typesCombo_;
  QLineEdit* valueEdit_;
  QComboBox* boolValueEdit_;
  QListWidget* valueListEdit_;
  QTableWidget* valueTableEdit_;
  QPushButton* addItemButton_;
  QPushButton* removeItemButton_;
  core::NDbKValue key_;
};
}  // namespace gui
}  // namespace fastonosql
