/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "gui/hash_table_model.h"

#include <QToolButton>

#include <common/macros.h>
#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTableModel::HashTableModel(QObject* parent) : common::qt::gui::TableModel(parent) {
  data_.push_back(createEmptyRow());
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
    if (col == KeyValueTableItem::kKey) {
      result = node->key();
    } else if (col == KeyValueTableItem::kValue) {
      result = node->value();
    } else if (col == KeyValueTableItem::kAction) {
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

    if (col == KeyValueTableItem::kKey) {
      node->setKey(value.toString());
    } else if (col == KeyValueTableItem::kValue) {
      node->setValue(value.toString());
    }
  }

  return false;
}

Qt::ItemFlags HashTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return 0;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();

  if (col == KeyValueTableItem::kKey || col == KeyValueTableItem::kValue) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant HashTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == KeyValueTableItem::kKey) {
      return translations::trKey;
    } else if (section == KeyValueTableItem::kValue) {
      return translations::trValue;
    } else if (section == KeyValueTableItem::kAction) {
      return translations::trAction;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int HashTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);
  return KeyValueTableItem::kCountColumns;
}

void HashTableModel::clear() {
  beginResetModel();
  for (size_t i = 0; i < data_.size(); ++i) {
    delete data_[i];
  }
  data_.clear();
  data_.push_back(createEmptyRow());
  endResetModel();
}

common::ArrayValue* HashTableModel::arrayValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    std::string key = common::ConvertToString(node->key());
    ar->AppendString(key);
  }

  return ar;
}

common::SetValue* HashTableModel::setValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::SetValue* ar = common::Value::CreateSetValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    std::string key = common::ConvertToString(node->key());
    ar->Insert(key);
  }

  return ar;
}

common::ZSetValue* HashTableModel::zsetValue() const {
  if (data_.size() < 2) {
    return nullptr;
  }

  common::ZSetValue* ar = common::Value::CreateZSetValue();
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    std::string key = common::ConvertToString(node->key());
    std::string val = common::ConvertToString(node->value());
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
    std::string key = common::ConvertToString(node->key());
    std::string val = common::ConvertToString(node->value());
    ar->Insert(key, val);
  }

  return ar;
}

void HashTableModel::insertRow(const QString& key, const QString& value) {
  size_t size = data_.size();
  beginInsertRows(QModelIndex(), size, size);
  data_.insert(data_.begin() + size - 1, new KeyValueTableItem(key, value, KeyValueTableItem::RemoveAction));
  KeyValueTableItem* last = static_cast<KeyValueTableItem*>(data_.back());
  last->setKey(QString());
  last->setValue(QString());
  endInsertRows();
}

void HashTableModel::removeRow(int row) {
  if (row == -1) {
    return;
  }

  beginRemoveRows(QModelIndex(), row, row);
  common::qt::gui::TableItem* child = data_[row];
  data_.erase(data_.begin() + row);
  delete child;
  endRemoveRows();
}

common::qt::gui::TableItem* HashTableModel::createEmptyRow() const {
  return new KeyValueTableItem(QString(), QString(), KeyValueTableItem::AddAction);
}

}  // namespace gui
}  // namespace fastonosql
