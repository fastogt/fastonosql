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

#include "gui/keys_table_model.h"

#include <QColor>
#include <QIcon>

#include <common/qt/utils_qt.h>

#include "translations/global.h"

#include "gui/gui_factory.h"
#include "gui/key_table_item.h"

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
  if (role == Qt::DecorationRole && col == KeyTableItem::kKey) {
    return GuiFactory::GetInstance().GetIcon(node->GetType());
  }

  if (role == Qt::TextColorRole && col == KeyTableItem::kType) {
    return QColor(Qt::gray);
  }

  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == KeyTableItem::kKey) {
      result = node->GetKeyString();
    } else if (col == KeyTableItem::kType) {
      result = node->GetTypeText();
    } else if (col == KeyTableItem::kTTL) {
      result = node->GetTTL();
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
    if (column == KeyTableItem::kKey) {
    } else if (column == KeyTableItem::kTTL) {
      bool is_ok = false;
      int newValue = value.toInt(&is_ok);
      if (is_ok && newValue != node->GetTTL()) {
        core::NDbKValue dbv = node->GetDBV();
        emit changedTTL(dbv, newValue);
      }
    }
  }

  return false;
}

Qt::ItemFlags KeysTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return 0;
  }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  int col = index.column();
  KeyTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyTableItem*>(index);
  if (node && col == KeyTableItem::kTTL) {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

QVariant KeysTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == KeyTableItem::kKey) {
      return translations::trKey;
    } else if (section == KeyTableItem::kType) {
      return translations::trType;
    } else if (section == KeyTableItem::kTTL) {
      return "TTL";
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int KeysTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return KeyTableItem::kCountColumns;
}

void KeysTableModel::updateKey(const core::NKey& key) {
  for (size_t i = 0; i < data_.size(); ++i) {
    KeyTableItem* it = dynamic_cast<KeyTableItem*>(data_[i]);  // +
    if (!it) {
      continue;
    }

    core::NDbKValue dbv = it->GetDBV();
    const core::NKey dbv_key = dbv.GetKey();
    if (dbv_key.GetKey() == key.GetKey()) {
      it->SetKey(key);
      updateItem(index(i, KeyTableItem::kKey, QModelIndex()), index(i, KeyTableItem::kTTL, QModelIndex()));
      break;
    }
  }
}

void KeysTableModel::clear() {
  beginResetModel();
  for (size_t i = 0; i < data_.size(); ++i) {
    delete data_[i];
  }
  data_.clear();
  endResetModel();
}

}  // namespace gui
}  // namespace fastonosql
