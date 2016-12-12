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

#include "gui/hash_table_model.h"

#include <QToolButton>

#include <common/macros.h>
#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include <gui/gui_factory.h>

#include "translations/global.h"

namespace fastonosql {
namespace gui {

KeyValueTableItem::KeyValueTableItem(const QString& key, const QString& value)
    : TableItem(), key_(key), value_(value) {}

QString KeyValueTableItem::key() const {
  return key_;
}

void KeyValueTableItem::setKey(const QString& key) {
  key_ = key;
}

QString KeyValueTableItem::value() const {
  return value_;
}

void KeyValueTableItem::setValue(const QString& val) {
  value_ = val;
}

HashTableModel::HashTableModel(QObject* parent) : common::qt::gui::TableModel(parent) {}

HashTableModel::~HashTableModel() {}

QVariant HashTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  int row = index.row();
  int col = index.column();
  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == KeyValueTableItem::kKey) {
      KeyValueTableItem* node =
          common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
      result = node->key();
    } else if (col == KeyValueTableItem::kValue) {
      KeyValueTableItem* node =
          common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
      result = node->value();
    } else if (col == KeyValueTableItem::kAction) {
      if (row == data_.size()) {
        result = GuiFactory::instance().addIcon();
      } else {
        result = GuiFactory::instance().removeIcon();
      }
    }
  }

  return result;
}

bool HashTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();
    int col = index.column();

    // handle last row

    KeyValueTableItem* node =
        common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);

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
  int row = index.row();
  int col = index.column();

  KeyValueTableItem* node =
      common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);

  if (node && (col == KeyValueTableItem::kKey || col == KeyValueTableItem::kValue)) {
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

common::ZSetValue* HashTableModel::zsetValue() const {
  common::ZSetValue* ar = common::Value::createZSetValue();
  DCHECK(data_.size() >= 1);
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    std::string key = common::ConvertToString(node->key());
    std::string val = common::ConvertToString(node->value());
    ar->insert(key, val);
  }

  return ar;
}

common::HashValue* HashTableModel::hashValue() const {
  common::HashValue* ar = common::Value::createHashValue();
  DCHECK(data_.size() >= 1);
  for (size_t i = 0; i < data_.size() - 1; ++i) {
    KeyValueTableItem* node = static_cast<KeyValueTableItem*>(data_[i]);
    std::string key = common::ConvertToString(node->key());
    std::string val = common::ConvertToString(node->value());
    ar->insert(key, val);
  }

  return ar;
}

void HashTableModel::insertRow(const QString& key, const QString& value) {
  beginInsertRows(QModelIndex(), data_.size(), data_.size());
  data_.insert(data_.begin(), new KeyValueTableItem(key, value));
  endInsertRows();
}

KeyValueTableItem* HashTableModel::createEmptyRow() const {
  return new KeyValueTableItem(QString(), QString());
}

}  // namespace gui
}  // namespace fastonosql
