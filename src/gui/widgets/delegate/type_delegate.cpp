/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/delegate/type_delegate.h"

#include <string>

#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include <fastonosql/core/value.h>

#include "gui/models/items/fasto_common_item.h"
#include "gui/widgets/hash_type_widget.h"
#include "gui/widgets/list_type_view.h"

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {

TypeDelegate::TypeDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QSize TypeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  UNUSED(option);
  UNUSED(index);
  return QSize(row_height, row_height);
}

QWidget* TypeDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
  FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!node) {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  common::Value::Type t = node->type();
  if (t == common::Value::TYPE_INTEGER || t == common::Value::TYPE_UINTEGER) {
    QSpinBox* editor = new QSpinBox(parent);
    editor->setRange(INT32_MIN, INT32_MAX);
    return editor;
  } else if (t == common::Value::TYPE_BOOLEAN) {
    QComboBox* editor = new QComboBox(parent);
    editor->addItem("true");
    editor->addItem("false");
    return editor;
  } else if (t == common::Value::TYPE_STRING) {
    QLineEdit* editor = new QLineEdit(parent);
    return editor;
  } else if (t == common::Value::TYPE_ARRAY || t == common::Value::TYPE_SET) {
    ListTypeView* editor = new ListTypeView(parent);
    editor->horizontalHeader()->hide();
    editor->verticalHeader()->hide();
    return editor;
  } else if (t == common::Value::TYPE_ZSET || t == common::Value::TYPE_HASH) {
    HashTypeView* editor = new HashTypeView(parent);
    editor->horizontalHeader()->hide();
    editor->verticalHeader()->hide();
    return editor;
  } else {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
}

void TypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
  FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!node) {
    return;
  }

  core::NDbKValue dbv = node->dbv();
  core::NValue val = dbv.GetValue();
  common::Value::Type t = node->type();
  if (t == common::Value::TYPE_INTEGER) {
    int value = 0;
    if (val->GetAsInteger(&value)) {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      spinBox->setValue(value);
    }
  } else if (t == common::Value::TYPE_UINTEGER) {
    unsigned int value = 0;
    if (val->GetAsUInteger(&value)) {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      spinBox->setValue(static_cast<int>(value));
    }
  } else if (t == common::Value::TYPE_BOOLEAN) {
    bool value;
    if (val->GetAsBoolean(&value)) {
      QComboBox* combobox = static_cast<QComboBox*>(editor);
      combobox->setCurrentIndex(value ? 0 : 1);
    }
  } else if (t == common::Value::TYPE_STRING) {
    common::Value::string_t value;
    if (val->GetAsString(&value)) {
      QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
      QString qvalue;
      common::ConvertFromBytes(value, &qvalue);
      lineedit->setText(qvalue);
    }
  } else if (t == core::JsonValue::TYPE_JSON) {
    common::Value::string_t value;
    if (val->GetAsString(&value)) {
      QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
      QString qvalue;
      common::ConvertFromBytes(value, &qvalue);
      lineedit->setText(qvalue);
    }
  } else if (t == common::Value::TYPE_ARRAY) {
    common::ArrayValue* arr = nullptr;
    if (val->GetAsList(&arr)) {
      ListTypeView* listwidget = static_cast<ListTypeView*>(editor);
      for (auto it = arr->begin(); it != arr->end(); ++it) {
        const auto val = core::ConvertValue(*it, core::NValue::default_delimiter);
        if (val.empty()) {
          continue;
        }

        listwidget->insertRow(val);
      }
    }
  } else if (t == common::Value::TYPE_SET) {
    common::SetValue* set = nullptr;
    if (val->GetAsSet(&set)) {
      ListTypeView* listwidget = static_cast<ListTypeView*>(editor);
      for (auto it = set->begin(); it != set->end(); ++it) {
        const auto val = core::ConvertValue(*it, core::NValue::default_delimiter);
        if (val.empty()) {
          continue;
        }

        listwidget->insertRow(val);
      }
    }
  } else if (t == common::Value::TYPE_ZSET) {
    common::ZSetValue* zset = nullptr;
    if (val->GetAsZSet(&zset)) {
      HashTypeView* hashwidget = static_cast<HashTypeView*>(editor);
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        common::Value::string_t key_str = core::ConvertValue(key, core::NValue::default_delimiter);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        common::Value::string_t value_str = core::ConvertValue(value, core::NValue::default_delimiter);
        if (value_str.empty()) {
          continue;
        }

        hashwidget->insertRow(key_str, value_str);
      }
    }
  } else if (t == common::Value::TYPE_HASH) {
    common::HashValue* hash = nullptr;
    if (val->GetAsHash(&hash)) {
      HashTypeView* hashwidget = static_cast<HashTypeView*>(editor);
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        common::Value::string_t key_str = core::ConvertValue(key, core::NValue::default_delimiter);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        common::Value::string_t value_str = core::ConvertValue(value, core::NValue::default_delimiter);
        if (value_str.empty()) {
          continue;
        }

        hashwidget->insertRow(key_str, value_str);
      }
    }
  } else {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void TypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
  FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!node) {
    return;
  }

  const common::Value::Type type = node->type();
  if (type == common::Value::TYPE_INTEGER) {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    int value = spinBox->value();
    core::NValue val(common::Value::CreateIntegerValue(value));
    QVariant var = QVariant::fromValue(val);
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_UINTEGER) {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    int value = spinBox->value();
    core::NValue val(common::Value::CreateUIntegerValue(static_cast<unsigned int>(value)));
    QVariant var = QVariant::fromValue(val);
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_BOOLEAN) {
    QComboBox* combobox = static_cast<QComboBox*>(editor);
    int cindex = combobox->currentIndex();
    core::NValue val(common::Value::CreateBooleanValue(cindex == 0));
    QVariant var = QVariant::fromValue(val);
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_STRING) {
    QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
    QString text = lineedit->text();
    if (text.isEmpty()) {
      return;
    }

    common::StringValue* string = common::Value::CreateStringValue(common::ConvertToCharBytes(text));
    QVariant var = QVariant::fromValue(core::NValue(string));
    model->setData(index, var, Qt::EditRole);
  } else if (type == core::JsonValue::TYPE_JSON) {
    QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
    QString text = lineedit->text();
    if (text.isEmpty()) {
      return;
    }

    core::JsonValue* string = new core::JsonValue(common::ConvertToCharBytes(text));
    QVariant var = QVariant::fromValue(core::NValue(string));
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_ARRAY) {
    ListTypeView* listwidget = static_cast<ListTypeView*>(editor);
    common::ArrayValue* arr = listwidget->arrayValue();
    if (!arr) {
      return;
    }

    QVariant var = QVariant::fromValue(core::NValue(arr));
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_SET) {
    ListTypeView* listwidget = static_cast<ListTypeView*>(editor);
    common::SetValue* set = listwidget->setValue();
    if (!set) {
      return;
    }

    QVariant var = QVariant::fromValue(core::NValue(set));
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_ZSET) {
    HashTypeView* hashwidget = static_cast<HashTypeView*>(editor);
    common::ZSetValue* zset = hashwidget->zsetValue();
    if (!zset) {
      return;
    }

    QVariant var = QVariant::fromValue(core::NValue(zset));
    model->setData(index, var, Qt::EditRole);
  } else if (type == common::Value::TYPE_HASH) {
    HashTypeView* hashwidget = static_cast<HashTypeView*>(editor);
    common::HashValue* hash = hashwidget->hashValue();
    if (!hash) {
      return;
    }

    QVariant var = QVariant::fromValue(core::NValue(hash));
    model->setData(index, var, Qt::EditRole);
  } else {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

void TypeDelegate::updateEditorGeometry(QWidget* editor,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
  UNUSED(index);

  editor->setGeometry(option.rect);
}

}  // namespace gui
}  // namespace fastonosql
