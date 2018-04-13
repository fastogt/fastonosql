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

#include <QWidget>

class QAbstractItemModel;  // lines 25-25
class QPushButton;         // lines 24-24
class QComboBox;           // lines 23-23
class QLabel;

namespace fastonosql {
namespace gui {

class FastoEditorOutput;
class FastoTextView : public QWidget {
  Q_OBJECT
 public:
  FastoTextView(QWidget* parent = Q_NULLPTR);

  void setModel(QAbstractItemModel* model);

 private Q_SLOTS:
  void viewChange(int index);
  void textChange();
  void saveChanges();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  FastoEditorOutput* editor_;
  QLabel* views_label_;
  QComboBox* views_combo_box_;
  QPushButton* save_change_button_;
};

}  // namespace gui
}  // namespace fastonosql
