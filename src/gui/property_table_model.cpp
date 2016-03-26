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

#include "gui/property_table_model.h"

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

PropertyTableItem::PropertyTableItem(const QString& key, const QString& value)
  : key_(key), value_(value) {
}

PropertyTableModel::PropertyTableModel(QObject* parent)
  : TableModel(parent) {
}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const {
  QVariant result;

  if (!index.isValid())
    return result;

  PropertyTableItem* node = common::utils_qt::item<PropertyTableItem*>(index);

  if (!node)
    return result;

  int col = index.column();

  if (role == Qt::DisplayRole) {
    if (col == PropertyTableItem::eKey) {
      result = node->key_;
    } else if (col == PropertyTableItem::eValue) {
      result = node->value_;
    }
  }
  return result;
}

bool PropertyTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    int column = index.column();
    PropertyTableItem* node = common::utils_qt::item<PropertyTableItem*>(index);

    if (!node)
      return false;

     if (column == PropertyTableItem::eKey) {
     } else if (column == PropertyTableItem::eValue) {
       const QString&newValue = value.toString();
       if (newValue != node->value_) {
         PropertyType pr;
         pr.first = common::convertToString(node->key_);
         pr.second = common::convertToString(newValue);
         emit changedProperty(pr);
       }
     }
  }

  return false;
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags result = 0;
  if (index.isValid()) {
    result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    int col = index.column();
    if (col == PropertyTableItem::eValue) {
      result |= Qt::ItemIsEditable;
    }
  }
  return result;
}

QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == PropertyTableItem::eKey) {
      return translations::trKey;
    } else if (section == PropertyTableItem::eValue) {
      return translations::trValue;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int PropertyTableModel::columnCount(const QModelIndex& parent) const {
  return PropertyTableItem::eCountColumns;
}


void PropertyTableModel::changeProperty(const PropertyType& pr) {
  const QString key = common::convertFromString<QString>(pr.first);
  for (size_t i = 0; i < data_.size(); ++i) {
    PropertyTableItem* it = dynamic_cast<PropertyTableItem*>(data_[i]);
    if (it && it->key_ == key) {
      it->value_ = common::convertFromString<QString>(pr.second);
      emit dataChanged(index(i, 0), index(i, 1));
      break;
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
