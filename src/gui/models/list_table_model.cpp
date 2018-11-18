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

#include "gui/models/list_table_model.h"

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/models/items/value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

ListTableModel::ListTableModel(QObject* parent) : common::qt::gui::TableModel(parent), first_column_name_() {
  insertItem(createEmptyRow());
}

ListTableModel::~ListTableModel() {}

QVariant ListTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  // int row = index.row();
  int col = index.column();
  ValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, ValueTableItem*>(index);
  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == kValue) {
      result = node->value();
    } else if (col == kAction) {
      result = node->actionState();
    }
  }

  return result;
}

bool ListTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    // int row = index.row();
    int col = index.column();

    ValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, ValueTableItem*>(index);

    if (!node) {
      return false;
    }

    if (col == kValue) {
      node->setValue(value.toString());
    }
  }

  return false;
}

Qt::ItemFlags ListTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();

  if (col == kValue) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant ListTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == kValue) {
      return first_column_name_;
    } else if (section == kAction) {
      return translations::trAction;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int ListTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);
  return kCountColumns;
}

void ListTableModel::clear() {
  beginResetModel();
  clearData();
  insertItem(createEmptyRow());
  endResetModel();
}

common::ArrayValue* ListTableModel::arrayValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    ValueTableItem* node = static_cast<ValueTableItem*>(data_[i]);
    common::Value::string_t key = common::ConvertToCharBytes(node->value());
    ar->AppendString(key);
  }

  return ar;
}

common::SetValue* ListTableModel::setValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::SetValue* ar = common::Value::CreateSetValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    ValueTableItem* node = static_cast<ValueTableItem*>(data_[i]);
    common::Value::string_t key = common::ConvertToCharBytes(node->value());
    ar->Insert(key);
  }

  return ar;
}

void ListTableModel::insertRow(const QString& value) {
  size_t size = data_.size();
  beginInsertRows(QModelIndex(), size, size);
  data_.insert(data_.begin() + size - 1, new ValueTableItem(value, ValueTableItem::RemoveAction));
  ValueTableItem* last = static_cast<ValueTableItem*>(data_.back());
  last->setValue(QString());
  endInsertRows();
}

void ListTableModel::removeRow(int row) {
  if (row < 0) {
    return;
  }

  size_t stabled_index_row = static_cast<size_t>(row);
  beginRemoveRows(QModelIndex(), row, row);
  common::qt::gui::TableItem* child = data_[stabled_index_row];
  data_.erase(data_.begin() + row);
  delete child;
  endRemoveRows();
}

void ListTableModel::setFirstColumnName(const QString& name) {
  first_column_name_ = name;
}

common::qt::gui::TableItem* ListTableModel::createEmptyRow() const {
  return new ValueTableItem(QString(), ValueTableItem::AddAction);
}

}  // namespace gui
}  // namespace fastonosql
