/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/widgets/list_type_widget.h"

#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include "gui/widgets/fasto_viewer.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

ListTypeWidget::ListTypeWidget(QWidget* parent)
    : base_class(parent), view_(nullptr), more_less_button_(nullptr), value_edit_(nullptr) {
  view_ = new ListTypeView;
  more_less_button_ = new QPushButton;
  value_edit_ = createWidget<FastoViewer>();

  VERIFY(connect(view_, &ListTypeView::dataChangedSignal, this, &ListTypeWidget::dataChangedSignal));
  VERIFY(connect(view_, &ListTypeView::rowChanged, this, &ListTypeWidget::valueUpdate, Qt::DirectConnection));
  VERIFY(connect(more_less_button_, &QPushButton::clicked, this, &ListTypeWidget::toggleVisibleValueView));

  // controls
  QVBoxLayout* viewer_layout = new QVBoxLayout;
  viewer_layout->setContentsMargins(0, 0, 0, 0);
  viewer_layout->addWidget(view_);
  viewer_layout->addWidget(value_edit_);

  // options
  QVBoxLayout* options_layout = new QVBoxLayout;
  QSplitter* splitter = new QSplitter(Qt::Vertical);
  options_layout->addWidget(splitter);
  options_layout->addWidget(more_less_button_);

  // main
  QHBoxLayout* main = new QHBoxLayout;
  main->setContentsMargins(0, 0, 0, 0);
  main->addLayout(viewer_layout);
  main->addLayout(options_layout);
  setLayout(main);

  // sync
  value_edit_->setVisible(false);
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

void ListTypeWidget::toggleVisibleValueView() {
  value_edit_->setVisible(!value_edit_->isVisible());
  syncMoreButton();
}

void ListTypeWidget::syncMoreButton() {
  more_less_button_->setText(value_edit_->isVisible() ? translations::trLess : translations::trMore);
}

void ListTypeWidget::retranslateUi() {
  syncMoreButton();
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
