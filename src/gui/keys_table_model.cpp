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

#include "gui/keys_table_model.h"

#include <stddef.h>  // for size_t
#include <vector>    // for vector

#include <QIcon>

#include <common/macros.h>             // for CHECK, UNUSED
#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/qt/utils_qt.h>        // for item

#include <common/qt/gui/base/table_item.h>   // for TableItem
#include <common/qt/gui/base/table_model.h>  // for TableModel

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trKey, trTTL, trType

namespace fastonosql {
namespace gui {

KeyTableItem::KeyTableItem(const core::NDbKValue& dbv) : dbv_(dbv) {}

QString KeyTableItem::keyString() const {
  QString qkey;
  const core::NKey key = dbv_.GetKey();
  const core::key_t raw_key = key.GetKey();
  common::ConvertFromString(raw_key.ToString(), &qkey);
  return qkey;
}

QString KeyTableItem::typeText() const {
  QString qtype;
  common::ConvertFromString(common::Value::GetTypeName(dbv_.GetType()), &qtype);
  return qtype;
}

core::ttl_t KeyTableItem::ttl() const {
  core::NKey key = dbv_.GetKey();
  return key.GetTTL();
}

common::Value::Type KeyTableItem::type() const {
  return dbv_.GetType();
}

core::NDbKValue KeyTableItem::dbv() const {
  return dbv_;
}

void KeyTableItem::setDbv(const core::NDbKValue& val) {
  dbv_ = val;
}

core::NKey KeyTableItem::key() const {
  return dbv_.GetKey();
}

void KeyTableItem::setKey(const core::NKey& key) {
  dbv_.SetKey(key);
}

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
    return GuiFactory::Instance().icon(node->type());
  }

  if (role == Qt::TextColorRole && col == KeyTableItem::kType) {
    return QColor(Qt::gray);
  }

  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == KeyTableItem::kKey) {
      result = node->keyString();
    } else if (col == KeyTableItem::kType) {
      result = node->typeText();
    } else if (col == KeyTableItem::kTTL) {
      result = node->ttl();
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
      bool isOk = false;
      int newValue = value.toInt(&isOk);
      if (isOk && newValue != node->ttl()) {
        core::NDbKValue dbv = node->dbv();
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
      return translations::trTTL;
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

    core::NDbKValue dbv = it->dbv();
    const core::NKey dbv_key = dbv.GetKey();
    if (dbv_key.GetKey() == key.GetKey()) {
      it->setKey(key);
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
