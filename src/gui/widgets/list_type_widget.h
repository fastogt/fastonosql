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

#include "gui/widgets/list_type_view.h"

class QPushButton;

namespace fastonosql {
namespace gui {

class FastoViewer;

class ListTypeWidget : public QWidget {
  Q_OBJECT

 public:
  typedef QWidget base_class;
  explicit ListTypeWidget(QWidget* parent = Q_NULLPTR);

  common::ArrayValue* arrayValue() const;  // alocate memory
  common::SetValue* setValue() const;      // alocate memory

  void insertRow(const ListTypeView::row_t& value);
  void clear();

  ListTypeView::Mode currentMode() const;
  void setCurrentMode(ListTypeView::Mode mode);

 Q_SIGNALS:
  void dataChangedSignal();

 private Q_SLOTS:
  void valueUpdate(const ListTypeView::row_t& value);
  void toggleVisibleValueView();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();
  void syncMoreButton();

  ListTypeView* view_;
  QPushButton* more_less_button_;
  FastoViewer* value_edit_;
};

}  // namespace gui
}  // namespace fastonosql
