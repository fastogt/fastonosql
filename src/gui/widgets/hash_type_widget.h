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

#include "gui/widgets/hash_type_view.h"

class QPushButton;

namespace fastonosql {
namespace gui {

class FastoViewer;

class HashTypeWidget : public QWidget {
  Q_OBJECT

 public:
  typedef QWidget base_class;
  explicit HashTypeWidget(QWidget* parent = Q_NULLPTR);

  void insertRow(const HashTypeView::key_t& key, const HashTypeView::value_t& value);
  void clear();

  common::ZSetValue* zsetValue() const;  // alocate memory
  common::HashValue* hashValue() const;  // alocate memory

  HashTypeView::Mode currentMode() const;
  void setCurrentMode(HashTypeView::Mode mode);

 Q_SIGNALS:
  void dataChangedSignal();

 private Q_SLOTS:
  void valueUpdate(const HashTypeView::key_t& key, const HashTypeView::value_t& value);
  void toggleVisibleValueView();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();
  void syncMoreButton();

  HashTypeView* view_;
  QPushButton* more_less_button_;
  FastoViewer* key_edit_;
  FastoViewer* value_edit_;
};

}  // namespace gui
}  // namespace fastonosql
