/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#pragma once

#include <QDialog>

namespace fastonosql {
namespace gui {

class BaseDialog : public QDialog {
  Q_OBJECT

 public:
  typedef QDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  ~BaseDialog() override;

 protected:
  explicit BaseDialog(const QString& title, QWidget* parent = Q_NULLPTR);

  void changeEvent(QEvent* ev) override;

  virtual void retranslateUi();
  virtual void updateFont();
};

template <typename T, typename... Args>
inline T* createDialog(Args&&... args) {
  T* dialog = new T(std::forward<Args>(args)...);
  dialog->updateFont();
  dialog->retranslateUi();  // protected
  return dialog;
}

}  // namespace gui
}  // namespace fastonosql
