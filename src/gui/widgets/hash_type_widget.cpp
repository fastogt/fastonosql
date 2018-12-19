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

#include "gui/widgets/hash_type_widget.h"

#include <QVBoxLayout>

#include "gui/widgets/fasto_viewer.h"

namespace fastonosql {
namespace gui {

HashTypeWidget::HashTypeWidget(QWidget* parent) : base_class(parent), view_(nullptr) {
  view_ = new HashTypeView;
  value_edit_ = new FastoViewer;
  key_edit_ = new FastoViewer;

  VERIFY(connect(view_, &HashTypeView::dataChangedSignal, this, &HashTypeWidget::dataChangedSignal));
  VERIFY(connect(view_, &HashTypeView::rowChanged, this, &HashTypeWidget::valueUpdate, Qt::DirectConnection));

  QVBoxLayout* main = new QVBoxLayout;
  QHBoxLayout* kv_layout = new QHBoxLayout;
  kv_layout->addWidget(key_edit_);
  kv_layout->addWidget(value_edit_);

  main->setContentsMargins(0, 0, 0, 0);
  main->addWidget(view_);
  main->addLayout(kv_layout);
  setLayout(main);
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

}  // namespace gui
}  // namespace fastonosql
