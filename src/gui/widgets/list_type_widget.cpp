/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/macros.h>
#include <common/qt/utils_qt.h>

#include "gui/action_cell_delegate.h"
#include "gui/hash_table_model.h"
#include "gui/key_value_table_item.h"

namespace fastonosql {
namespace gui {

ListTypeWidget::ListTypeWidget(QWidget* parent) : QTableView(parent) {
  model_ = new HashTableModel(this);
  setModel(model_);

  setColumnHidden(KeyValueTableItem::kValue, true);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &ListTypeWidget::addRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &ListTypeWidget::removeRow));

  setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);
}

common::ArrayValue* ListTypeWidget::arrayValue() const {
  return model_->arrayValue();
}

common::SetValue* ListTypeWidget::setValue() const {
  return model_->setValue();
}

void ListTypeWidget::insertRow(const QString& first) {
  model_->insertRow(first, QString());
}

void ListTypeWidget::clear() {
  model_->clear();
}

void ListTypeWidget::addRow(const QModelIndex& index) {
  KeyValueTableItem* node =
      common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  model_->insertRow(node->key(), node->value());
}

void ListTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
}

}  // namespace gui
}  // namespace fastonosql
