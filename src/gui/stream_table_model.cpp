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

#include "gui/stream_table_model.h"

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

StreamTableModel::StreamTableModel(QObject* parent) : common::qt::gui::TableModel(parent) {
  data_.push_back(createEmptyEntry());
}

StreamTableModel::~StreamTableModel() {}

QVariant StreamTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  // int row = index.row();
  const int col = index.column();
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

bool StreamTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
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

Qt::ItemFlags StreamTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();

  if (col == KeyValueTableItem::kKey || col == KeyValueTableItem::kValue) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant StreamTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == KeyValueTableItem::kKey) {
      return translations::trField;
    } else if (section == KeyValueTableItem::kValue) {
      return translations::trString;
    } else if (section == KeyValueTableItem::kAction) {
      return translations::trAction;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int StreamTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);
  return KeyValueTableItem::kCountColumns;
}

void StreamTableModel::clear() {
  beginResetModel();
  for (size_t i = 0; i < data_.size(); ++i) {
    delete data_[i];
  }
  data_.clear();
  data_.push_back(createEmptyEntry());
  endResetModel();
}

bool StreamTableModel::getStream(core::StreamValue::stream_id sid, core::StreamValue::Stream* out) const {
  if (!out || data_.size() < 2) {
    return false;
  }

  std::vector<core::StreamValue::Entry> entries;
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    const common::Value::string_t key = common::ConvertToCharBytes(node->key());
    const common::Value::string_t val = common::ConvertToCharBytes(node->value());
    entries.push_back(core::StreamValue::Entry{key, val});
  }

  *out = {sid, entries};
  return true;
}

void StreamTableModel::insertEntry(const QString& key, const QString& value) {
  const size_t size = data_.size();
  beginInsertRows(QModelIndex(), size, size);
  data_.insert(data_.begin() + size - 1, new KeyValueTableItem(key, value, KeyValueTableItem::RemoveAction));
  KeyValueTableItem* last = static_cast<KeyValueTableItem*>(data_.back());
  last->setKey(QString());
  last->setValue(QString());
  endInsertRows();
}

void StreamTableModel::removeEntry(int row) {
  if (row < 0) {
    return;
  }

  const size_t stabled_row_index = static_cast<size_t>(row);
  beginRemoveRows(QModelIndex(), row, row);
  common::qt::gui::TableItem* child = data_[stabled_row_index];
  data_.erase(data_.begin() + row);
  delete child;
  endRemoveRows();
}

common::qt::gui::TableItem* StreamTableModel::createEmptyEntry() const {
  return new KeyValueTableItem(QString(), QString(), KeyValueTableItem::AddAction);
}

}  // namespace gui
}  // namespace fastonosql
