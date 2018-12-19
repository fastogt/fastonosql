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

#include "gui/widgets/list_type_widget.h"

#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include "gui/widgets/fasto_viewer.h"

namespace fastonosql {
namespace gui {

ListTypeWidget::ListTypeWidget(QWidget* parent) : base_class(parent), view_(nullptr) {
  view_ = new ListTypeView;
  value_edit_ = new FastoViewer;

  VERIFY(connect(view_, &ListTypeView::dataChangedSignal, this, &ListTypeWidget::dataChangedSignal));
  VERIFY(connect(view_, &ListTypeView::rowChanged, this, &ListTypeWidget::valueUpdate, Qt::DirectConnection));

  QVBoxLayout* main = new QVBoxLayout;
  main->setContentsMargins(0, 0, 0, 0);
  main->addWidget(view_);
  main->addWidget(value_edit_);
  setLayout(main);
}

common::ArrayValue* ListTypeWidget::arrayValue() const {
  return view_->arrayValue();
}

common::SetValue* ListTypeWidget::setValue() const {
  return view_->setValue();
}

void ListTypeWidget::insertRow(const ListTypeView::row_t& value) {
  view_->insertRow(value);
}

void ListTypeWidget::clear() {
  view_->clear();
}

ListTypeView::Mode ListTypeWidget::currentMode() const {
  return view_->currentMode();
}

void ListTypeWidget::setCurrentMode(ListTypeView::Mode mode) {
  view_->setCurrentMode(mode);
}

void ListTypeWidget::valueUpdate(const ListTypeView::row_t& value) {
  value_edit_->clear();
  value_edit_->setText(value);
}

}  // namespace gui
}  // namespace fastonosql
