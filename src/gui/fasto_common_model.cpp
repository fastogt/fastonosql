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

#include "gui/fasto_common_model.h"

#include <stddef.h>  // for size_t

#include <string>  // for string

#include <QIcon>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/macros.h>             // for UNUSED
#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/utils_qt.h>        // for item
#include <common/value.h>              // for Value, make_value, etc

#include <common/qt/gui/base/tree_item.h>   // for TreeItem
#include <common/qt/gui/base/tree_model.h>  // for TreeModel

#include "core/db_key.h"  // for NDbKValue, NKey, NValue

#include "gui/fasto_common_item.h"  // for FastoCommonItem, etc
#include "gui/gui_factory.h"        // for GuiFactory

#include "translations/global.h"  // for trKey, trType, trValue

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {

FastoCommonModel::FastoCommonModel(QObject* parent) : TreeModel(parent) {}

QVariant FastoCommonModel::data(const QModelIndex& index, int role) const {
  QVariant result;

  if (!index.isValid()) {
    return result;
  }

  FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);

  if (!node) {
    return result;
  }

  if (role == Qt::FontRole) {
    return gui::GuiFactory::instance().font();
  }

  int col = index.column();

  if (role == Qt::DecorationRole && col == FastoCommonItem::eKey) {
    return GuiFactory::instance().icon(node->type());
  }

  if (role == Qt::TextColorRole && col == FastoCommonItem::eType) {
    return QColor(Qt::gray);
  }

  if (role == Qt::DisplayRole) {
    if (col == FastoCommonItem::eKey) {
      result = node->key();
    } else if (col == FastoCommonItem::eValue) {
      result = node->value();
    } else if (col == FastoCommonItem::eType) {
      std::string type_str = common::Value::toString(node->type());
      result = common::ConvertFromString<QString>(type_str);
    }
  }

  return result;
}

bool FastoCommonModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);

    if (!node) {
      return false;
    }

    int column = index.column();
    if (column == FastoCommonItem::eKey) {
    } else if (column == FastoCommonItem::eValue) {
      if (value.canConvert<core::NValue>()) {
        core::NValue nvalue = value.value<core::NValue>();
        core::NValue nv = node->nvalue();
        if (!nvalue->equals(nv.get())) {
          core::NDbKValue dbv = node->dbv();
          dbv.SetValue(nvalue);
          emit changedValue(dbv);
        }
      } else {
        DNOTREACHED();
      }
    }
  }

  return false;
}

Qt::ItemFlags FastoCommonModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags result = 0;
  if (index.isValid()) {
    result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    int col = index.column();
    FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
    if (node && col == FastoCommonItem::eValue && !node->isReadOnly()) {
      result |= Qt::ItemIsEditable;
    }
  }
  return result;
}

QVariant FastoCommonModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == FastoCommonItem::eKey) {
      return translations::trKey + "/" + translations::trCommands;
    } else if (section == FastoCommonItem::eValue) {
      return translations::trValue + "/" + translations::trResult;
    } else {
      return translations::trType;
    }
  }

  return TreeModel::headerData(section, orientation, role);
}

int FastoCommonModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return FastoCommonItem::eCountColumns;
}

void FastoCommonModel::changeValue(const core::NDbKValue& value) {
  QModelIndex ind = index(0, 0, QModelIndex());
  if (!ind.isValid()) {
    return;
  }

  FastoCommonItem* child = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(ind);
  if (!child) {
    return;
  }

  common::qt::gui::TreeItem* root = child->parent();
  if (!root) {
    return;
  }

  const QString key = common::ConvertFromString<QString>(value.KeyString());
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));  // +
    if (!child) {
      continue;
    }

    if (child->key() == key) {
      child->setValue(value.Value());
      updateItem(index(i, FastoCommonItem::eValue, QModelIndex()),
                 index(i, FastoCommonItem::eType, QModelIndex()));
      break;
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
