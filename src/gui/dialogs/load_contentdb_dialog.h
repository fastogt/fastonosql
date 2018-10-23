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

class QLineEdit;  // lines 25-25
class QSpinBox;   // lines 26-26
class QLabel;

namespace fastonosql {
namespace gui {

class LoadContentDbDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_key_on_page = 1, max_key_on_page = 1000000, defaults_key = 1000, step_keys_on_page = defaults_key };

  explicit LoadContentDbDialog(const QString& title, const QIcon& icon, QWidget* parent = Q_NULLPTR);

  int count() const;
  QString pattern() const;

 public Q_SLOTS:
  virtual void accept() override;

 protected:
  virtual void changeEvent(QEvent* e) override;

 private:
  void retranslateUi();

  QLabel* keys_count_label_;
  QLabel* key_pattern_label_;
  QLineEdit* pattern_edit_;
  QSpinBox* count_spin_edit_;
};

}  // namespace gui
}  // namespace fastonosql
