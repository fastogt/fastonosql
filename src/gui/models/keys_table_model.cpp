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

#include "gui/models/keys_table_model.h"

#include <QColor>
#include <QIcon>

#include <common/qt/utils_qt.h>

#include "translations/global.h"

#include "gui/gui_factory.h"
#include "gui/models/items/key_table_item.h"

namespace fastonosql {
namespace gui {

KeysTableModel::KeysTableModel(QObject* parent) : TableModel(parent) {}

KeysTableModel::~KeysTableModel() {}

QVariant KeysTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  KeyTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyTableItem*>(index);
  if (!node) {
    return QVariant();
  }

  int col = index.column();
  if (role == Qt::DecorationRole && col == kKey) {
    return GuiFactory::GetInstance().icon(node->type());
  }

  if (role == Qt::TextColorRole && col == kType) {
    return QColor(Qt::gray);
  }

  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == kKey) {
      result = node->keyString();
    } else if (col == kType) {
      result = node->typeText();
    } else if (col == kTTL) {
      result = node->TTL();
    }
  }

  return result;
}

bool KeysTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    KeyTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyTableItem*>(index);

    if (!node) {
      return false;
    }

    int column = index.column();
    if (column == kKey) {
    } else if (column == kTTL) {
      bool is_ok = false;
      int newValue = value.toInt(&is_ok);
      if (is_ok && newValue != node->TTL()) {
        core::NDbKValue dbv = node->dbv();
        emit changedTTL(dbv, newValue);
      }
    }
  }

  return false;
}

Qt::ItemFlags KeysTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();
  KeyTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyTableItem*>(index);
  if (node && col == kTTL) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant KeysTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == kKey) {
      return translations::trKey;
    } else if (section == kType) {
      return translations::trType;
    } else if (section == kTTL) {
      return "TTL";
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int KeysTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return kCountColumns;
}

void KeysTableModel::insertKey(const core::NDbKValue& key) {
  insertItem(new KeyTableItem(key));
}

void KeysTableModel::updateKey(const core::NKey& key) {
  for (size_t i = 0; i < data_.size(); ++i) {
    KeyTableItem* it = dynamic_cast<KeyTableItem*>(data_[i]);  // +
    if (!it) {
      continue;
    }

    core::NDbKValue dbv = it->dbv();
    const core::NKey dbv_key = dbv.GetKey();
    if (dbv_key.GetKey() == key.GetKey()) {
      it->setKey(key);
      updateItem(index(i, kKey, QModelIndex()), index(i, kTTL, QModelIndex()));
      break;
    }
  }
}

void KeysTableModel::clear() {
  beginResetModel();
  clearData();
  endResetModel();
}

}  // namespace gui
}  // namespace fastonosql
