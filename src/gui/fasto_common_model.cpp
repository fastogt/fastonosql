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

#include <string>

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "translations/global.h"

#include "gui/fasto_common_item.h"
#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {

FastoCommonModel::FastoCommonModel(QObject* parent)
  : TreeModel(parent) {
}

QVariant FastoCommonModel::data(const QModelIndex& index, int role) const {
  QVariant result;

  if (!index.isValid()) {
    return result;
  }

  FastoCommonItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(index);

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
      result = common::ConvertFromString<QString>(common::Value::toString(node->type()));
    }
  }

  return result;
}

bool FastoCommonModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == Qt::EditRole) {
    FastoCommonItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(index);

    if (!node) {
      return false;
    }

    int column = index.column();
    if (column == FastoCommonItem::eKey) {
    } else if (column == FastoCommonItem::eValue) {
      QString newValue = value.toString();
      if (newValue != node->value()) {
        const std::string key = common::ConvertToString(node->key());
        const std::string value = common::ConvertToString(newValue);

        //  node->type() TODO: create according type
        common::ValueSPtr vs = common::make_value(common::Value::createStringValue(value));
        core::NValue val(vs);
        core::NDbKValue dbv(core::NKey(key), val);
        core::CommandKeySPtr com(new core::CommandCreateKey(dbv));
        emit changedValue(com);
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
      FastoCommonItem* node = common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(index);
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
      return translations::trKey;
    } else if (section == FastoCommonItem::eValue) {
      return translations::trValue;
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

  FastoCommonItem* child = common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(ind);
  if (!child) {
    return;
  }

  fasto::qt::gui::TreeItem* root = child->parent();
  if (!root) {
    return;
  }

  const QString key = common::ConvertFromString<QString>(value.keyString());
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));  // +
    if (!child) {
      continue;
    }

    if (child->key() == key) {
      child->setValue(value.value());
      updateItem(index(i, FastoCommonItem::eValue, QModelIndex()), index(i, FastoCommonItem::eType, QModelIndex()));
      break;
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
