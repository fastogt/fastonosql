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

#include "gui/property_table_model.h"

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>  // for item

#include "gui/property_table_item.h"

#include "translations/global.h"  // for trKey, trValue

namespace fastonosql {
namespace gui {

PropertyTableModel::PropertyTableModel(QObject* parent) : TableModel(parent) {}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  PropertyTableItem* node = common::qt::item<common::qt::gui::TableItem*, PropertyTableItem*>(index);
  if (!node) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    int col = index.column();
    if (col == PropertyTableItem::kKey) {
      return node->key();
    } else if (col == PropertyTableItem::kValue) {
      return node->value();
    }
  }
  return QVariant();
}

bool PropertyTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    PropertyTableItem* node = common::qt::item<common::qt::gui::TableItem*, PropertyTableItem*>(index);
    if (!node) {
      return false;
    }

    int column = index.column();
    if (column == PropertyTableItem::kKey) {
    } else if (column == PropertyTableItem::kValue) {
      QString new_value = value.toString();
      if (new_value != node->value()) {
        core::property_t pr = node->property();
        pr.second = common::ConvertToString(new_value);
        emit propertyChanged(pr);
      }
    }
  }

  return false;
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return 0;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();
  if (col == PropertyTableItem::kValue) {
    result |= Qt::ItemIsEditable;
  }
  return result;
}

QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == PropertyTableItem::kKey) {
      return translations::trKey;
    } else if (section == PropertyTableItem::kValue) {
      return translations::trValue;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int PropertyTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return PropertyTableItem::kColumnsCount;
}

void PropertyTableModel::changeProperty(const core::property_t& pr) {
  for (size_t i = 0; i < data_.size(); ++i) {
    PropertyTableItem* it = dynamic_cast<PropertyTableItem*>(data_[i]);  // +
    if (!it) {
      continue;
    }

    core::property_t prop = it->property();
    if (prop.first == pr.first) {
      it->setProperty(pr);
      updateItem(index(i, PropertyTableItem::kKey, QModelIndex()), index(i, PropertyTableItem::kValue, QModelIndex()));
      return;
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
