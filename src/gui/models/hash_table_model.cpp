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

#include "gui/models/hash_table_model.h"

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/models/items/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTableModel::HashTableModel(QObject* parent)
    : common::qt::gui::TableModel(parent), first_column_name_(), second_column_name_() {
  insertItem(createEmptyRow());
}

HashTableModel::~HashTableModel() {}

QVariant HashTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  // int row = index.row();
  int col = index.column();
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == kKey) {
      QString key;
      if (common::ConvertFromBytes(node->key(), &key)) {
        result = key;
      }
    } else if (col == kValue) {
      QString value;
      if (common::ConvertFromBytes(node->value(), &value)) {
        result = value;
      }
    } else if (col == kAction) {
      result = node->actionState();
    }
  }

  return result;
}

bool HashTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    // int row = index.row();
    int col = index.column();

    KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);

    if (!node) {
      return false;
    }

    if (col == kKey) {
      const QString val = value.toString();
      node->setKey(common::ConvertToCharBytes(val));
    } else if (col == kValue) {
      const QString val = value.toString();
      node->setValue(common::ConvertToCharBytes(val));
    }
  }

  return false;
}

Qt::ItemFlags HashTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();

  if (col == kKey || col == kValue) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant HashTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == kKey) {
      return first_column_name_;
    } else if (section == kValue) {
      return second_column_name_;
    } else if (section == kAction) {
      return translations::trAction;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int HashTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);
  return kCountColumns;
}

void HashTableModel::clear() {
  beginResetModel();
  clearData();
  insertItem(createEmptyRow());
  endResetModel();
}

common::ZSetValue* HashTableModel::zsetValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::ZSetValue* ar = common::Value::CreateZSetValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    common::Value::string_t key = node->key();
    common::Value::string_t val = node->value();
    ar->Insert(key, val);
  }

  return ar;
}

common::HashValue* HashTableModel::hashValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::HashValue* ar = common::Value::CreateHashValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    common::Value::string_t key = node->key();
    common::Value::string_t val = node->value();
    ar->Insert(key, val);
  }

  return ar;
}

void HashTableModel::insertRow(const key_t& key, const value_t& value) {
  size_t size = data_.size();
  beginInsertRows(QModelIndex(), size, size);
  data_.insert(data_.begin() + size - 1, new KeyValueTableItem(key, value, KeyValueTableItem::RemoveAction));
  KeyValueTableItem* last = static_cast<KeyValueTableItem*>(data_.back());
  last->setKey(key_t());
  last->setValue(value_t());
  endInsertRows();
}

void HashTableModel::removeRow(int row) {
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

void HashTableModel::setFirstColumnName(const QString& name) {
  first_column_name_ = name;
}

void HashTableModel::setSecondColumnName(const QString& name) {
  second_column_name_ = name;
}

common::qt::gui::TableItem* HashTableModel::createEmptyRow() const {
  return new KeyValueTableItem(KeyValueTableItem::key_t(), KeyValueTableItem::value_t(), KeyValueTableItem::AddAction);
}

}  // namespace gui
}  // namespace fastonosql
