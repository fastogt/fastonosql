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

#include "gui/models/hash_table_model.h"
#include "gui/models/items/key_value_table_item.h"
#include "gui/widgets/delegate/action_cell_delegate.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTypeView::HashTypeView(QWidget* parent) : QTableView(parent), model_(nullptr), mode_(kHash) {
  model_ = new HashTableModel(this);
  setModel(model_);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &HashTypeView::addRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &HashTypeView::removeRow));
  QAbstractItemDelegate* default_del = itemDelegate();
  VERIFY(connect(default_del, &QAbstractItemDelegate::closeEditor, this, &HashTypeView::dataChangedSignal));

  setItemDelegateForColumn(HashTableModel::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView* header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);

  // sync
  setCurrentMode(mode_);
}

HashTypeView::~HashTypeView() {}

void HashTypeView::insertRow(const row_t& first, const row_t& second) {
  model_->insertRow(first, second);
  emit dataChangedSignal();
}

void HashTypeView::clear() {
  model_->clear();
  emit dataChangedSignal();
}

common::ZSetValue* HashTypeView::zsetValue() const {
  return model_->zsetValue();
}

common::HashValue* HashTypeView::hashValue() const {
  return model_->hashValue();
}

HashTypeView::Mode HashTypeView::currentMode() const {
  return mode_;
}

void HashTypeView::setCurrentMode(Mode mode) {
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

void HashTypeView::addRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  insertRow(node->key(), node->value());
}

void HashTypeView::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
  emit dataChangedSignal();
}

}  // namespace gui
}  // namespace fastonosql
