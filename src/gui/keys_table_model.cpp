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

#include "gui/keys_table_model.h"

#include <stddef.h>  // for size_t
#include <vector>    // for vector

#include <QIcon>

#include "common/convert2string.h"  // for ConvertFromString
#include "common/macros.h"          // for CHECK, UNUSED
#include "common/qt/utils_qt.h"     // for item

#include "common/qt/gui/base/table_item.h"   // for TableItem
#include "common/qt/gui/base/table_model.h"  // for TableModel

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trKey, trTTL, trType

namespace fastonosql {
namespace gui {

KeyTableItem::KeyTableItem(const core::NDbKValue& key) : key_(key) {}

QString KeyTableItem::key() const {
  return common::ConvertFromString<QString>(key_.keyString());
}

QString KeyTableItem::typeText() const {
  return common::ConvertFromString<QString>(common::Value::toString(key_.type()));
}

core::ttl_t KeyTableItem::ttl() const {
  core::NKey key = key_.key();
  return key.ttl();
}

common::Value::Type KeyTableItem::type() const {
  return key_.type();
}

core::NDbKValue KeyTableItem::dbv() const {
  return key_;
}

void KeyTableItem::setDbv(const core::NDbKValue& val) {
  key_ = val;
}

KeysTableModel::KeysTableModel(QObject* parent) : TableModel(parent) {}

KeysTableModel::~KeysTableModel() {}

QVariant KeysTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  KeyTableItem* node = common::utils_qt::item<fasto::qt::gui::TableItem*, KeyTableItem*>(index);
  if (!node) {
    return QVariant();
  }

  int col = index.column();
  if (role == Qt::DecorationRole && col == KeyTableItem::kKey) {
    return GuiFactory::instance().icon(node->type());
  }

  if (role == Qt::TextColorRole && col == KeyTableItem::kType) {
    return QColor(Qt::gray);
  }

  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == KeyTableItem::kKey) {
      result = node->key();
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
    KeyTableItem* node = common::utils_qt::item<fasto::qt::gui::TableItem*, KeyTableItem*>(index);

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
        core::CommandKeySPtr com(new core::CommandChangeTTL(dbv, newValue));
        emit changedValue(com);
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
  KeyTableItem* node = common::utils_qt::item<fasto::qt::gui::TableItem*, KeyTableItem*>(index);
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

void KeysTableModel::changeValue(const core::NDbKValue& value) {
  const QString key = common::ConvertFromString<QString>(value.keyString());
  for (size_t i = 0; i < data_.size(); ++i) {
    KeyTableItem* it = dynamic_cast<KeyTableItem*>(data_[i]);  // +
    CHECK(it);
    if (it->key() == key) {
      it->setDbv(value);
      updateItem(index(i, KeyTableItem::kKey, QModelIndex()),
                 index(i, KeyTableItem::kTTL, QModelIndex()));
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
