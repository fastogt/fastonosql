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

#include <QHeaderView>

#include <common/qt/utils_qt.h>

#include "gui/action_cell_delegate.h"
#include "gui/hash_table_model.h"
#include "gui/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

namespace {
class ListTableModelInner : public HashTableModel {
 public:
  explicit ListTableModelInner(QObject* parent = Q_NULLPTR) : HashTableModel(parent) {}

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role != Qt::DisplayRole) {
      return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section == KeyValueTableItem::kKey) {
        return translations::trValue;
      } else if (section == KeyValueTableItem::kAction) {
        return translations::trAction;
      }
    }

    return TableModel::headerData(section, orientation, role);
  }
};
}  // namespace

ListTypeWidget::ListTypeWidget(QWidget* parent) : QTableView(parent) {
  model_ = new ListTableModelInner(this);
  setModel(model_);

  setColumnHidden(KeyValueTableItem::kValue, true);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &ListTypeWidget::addRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &ListTypeWidget::removeRow));
  QAbstractItemDelegate* default_del = itemDelegate();
  VERIFY(connect(default_del, &QAbstractItemDelegate::closeEditor, this, &ListTypeWidget::dataChanged));

  setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView* header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);
}

common::ArrayValue* ListTypeWidget::arrayValue() const {
  return model_->arrayValue();
}

common::SetValue* ListTypeWidget::setValue() const {
  return model_->setValue();
}

void ListTypeWidget::insertRow(const QString& first) {
  model_->insertRow(first, QString());
  emit dataChanged();
}

void ListTypeWidget::clear() {
  model_->clear();
  emit dataChanged();
}

void ListTypeWidget::addRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  model_->insertRow(node->key(), node->value());
  emit dataChanged();
}

void ListTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
  emit dataChanged();
}

}  // namespace gui
}  // namespace fastonosql
