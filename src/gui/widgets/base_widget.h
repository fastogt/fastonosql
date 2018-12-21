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

namespace fastonosql {
namespace gui {

class BaseWidget : public QWidget {
  Q_OBJECT

 public:
  typedef QWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  ~BaseWidget() override;

 protected:
  explicit BaseWidget(QWidget* parent = Q_NULLPTR);

  virtual void init();
  virtual void retranslateUi();

  void changeEvent(QEvent* ev) override;
};

template <typename T, typename... Args>
inline T* createWidget(Args&&... args) {
  T* widget = new T(std::forward<Args>(args)...);
  widget->init();
  widget->retranslateUi();  // protected
  return widget;
}

}  // namespace gui
}  // namespace fastonosql
