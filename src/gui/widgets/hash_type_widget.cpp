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

#include <QHeaderView>

#include <common/qt/utils_qt.h>

#include "gui/action_cell_delegate.h"
#include "gui/hash_table_model.h"
#include "gui/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTypeWidget::HashTypeWidget(QWidget* parent) : QTableView(parent), model_(nullptr), mode_(kHash) {
  model_ = new HashTableModel(this);
  setModel(model_);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &HashTypeWidget::addRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &HashTypeWidget::removeRow));
  QAbstractItemDelegate* default_del = itemDelegate();
  VERIFY(connect(default_del, &QAbstractItemDelegate::closeEditor, this, &HashTypeWidget::dataChangedSignal));

  setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView* header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);

  // sync
  setCurrentMode(mode_);
}

HashTypeWidget::~HashTypeWidget() {}

void HashTypeWidget::insertRow(const QString& first, const QString& second) {
  model_->insertRow(first, second);
  emit dataChangedSignal();
}

void HashTypeWidget::clear() {
  model_->clear();
  emit dataChangedSignal();
}

common::ZSetValue* HashTypeWidget::zsetValue() const {
  CHECK_EQ(mode_, kZset) << "Must be in zset mode.";
  return model_->zsetValue();
}

common::HashValue* HashTypeWidget::hashValue() const {
  CHECK_EQ(mode_, kHash) << "Must be in hash mode.";
  return model_->hashValue();
}

HashTypeWidget::Mode HashTypeWidget::currentMode() const {
  return mode_;
}

void HashTypeWidget::setCurrentMode(Mode mode) {
  mode_ = mode;
  if (mode == kHash) {
    model_->setFirstColumnName(translations::trField);
    model_->setSecondColumnName(translations::trValue);
  } else if (mode == kZset) {
    model_->setFirstColumnName(translations::trScore);
    model_->setSecondColumnName(translations::trMember);
  } else {
    NOTREACHED() << "Unhandled mode: " << mode;
  }
}

void HashTypeWidget::addRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  model_->insertRow(node->key(), node->value());
  emit dataChangedSignal();
}

void HashTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
  emit dataChangedSignal();
}

}  // namespace gui
}  // namespace fastonosql
