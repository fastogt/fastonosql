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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/hash_type_widget.h"

#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include "gui/widgets/fasto_viewer.h"
#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTypeWidget::HashTypeWidget(QWidget* parent)
    : base_class(parent), view_(nullptr), more_less_button_(nullptr), key_edit_(nullptr), value_edit_(nullptr) {
  view_ = new HashTypeView;
  more_less_button_ = new QPushButton;
  value_edit_ = createWidget<FastoViewer>();
  key_edit_ = createWidget<FastoViewer>();

  VERIFY(connect(view_, &HashTypeView::dataChangedSignal, this, &HashTypeWidget::dataChangedSignal));
  VERIFY(connect(view_, &HashTypeView::rowChanged, this, &HashTypeWidget::valueUpdate, Qt::DirectConnection));
  VERIFY(connect(more_less_button_, &QPushButton::clicked, this, &HashTypeWidget::toggleVisibleValueView));

  // controls
  QHBoxLayout* kv_layout = new QHBoxLayout;
  kv_layout->setContentsMargins(0, 0, 0, 0);
  kv_layout->addWidget(key_edit_);
  kv_layout->addWidget(value_edit_);

  QVBoxLayout* viewer_layout = new QVBoxLayout;
  viewer_layout->setContentsMargins(0, 0, 0, 0);
  viewer_layout->addWidget(view_);
  viewer_layout->addLayout(kv_layout);

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
  key_edit_->setVisible(false);
}

void HashTypeWidget::insertRow(const HashTypeView::key_t& key, const HashTypeView::value_t& value) {
  view_->insertRow(key, value);
}

void HashTypeWidget::clear() {
  view_->clear();
}

common::ZSetValue* HashTypeWidget::zsetValue() const {
  return view_->zsetValue();
}

common::HashValue* HashTypeWidget::hashValue() const {
  return view_->hashValue();
}

HashTypeView::Mode HashTypeWidget::currentMode() const {
  return view_->currentMode();
}

void HashTypeWidget::setCurrentMode(HashTypeView::Mode mode) {
  view_->setCurrentMode(mode);
}

void HashTypeWidget::valueUpdate(const HashTypeView::key_t& key, const HashTypeView::value_t& value) {
  key_edit_->clear();
  key_edit_->setText(key);

  value_edit_->clear();
  value_edit_->setText(value);
}

void HashTypeWidget::toggleVisibleValueView() {
  key_edit_->setVisible(!key_edit_->isVisible());
  value_edit_->setVisible(!value_edit_->isVisible());
  syncMoreButton();
}

void HashTypeWidget::syncMoreButton() {
  more_less_button_->setText(value_edit_->isVisible() ? translations::trLess : translations::trMore);
}

void HashTypeWidget::retranslateUi() {
  syncMoreButton();
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
