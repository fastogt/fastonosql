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

#include "gui/widgets/key_edit_widget.h"

#include <string>
#include <vector>

#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>  // for ConvertToString

#include <fastonosql/core/value.h>

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/models/hash_table_model.h"

#include "gui/widgets/fasto_viewer.h"
#include "gui/widgets/hash_type_widget.h"
#include "gui/widgets/list_type_widget.h"
#include "gui/widgets/stream_type_widget.h"

#include "translations/global.h"  // for trAddItem, trRemoveItem, etc

namespace fastonosql {
namespace gui {

KeyEditWidget::KeyEditWidget(const std::vector<common::Value::Type>& availible_types, QWidget* parent)
    : base_class(parent) {
  type_label_ = new QLabel;
  types_combo_box_ = new QComboBox;
  for (size_t i = 0; i < availible_types.size(); ++i) {
    common::Value::Type t = availible_types[i];
    QString type = core::GetTypeName(t);
    types_combo_box_->addItem(GuiFactory::GetInstance().icon(t), type, t);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(
      connect(types_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &KeyEditWidget::changeType));

  // key layout
  key_label_ = new QLabel;
  key_edit_ = new QLineEdit;
  key_edit_->setPlaceholderText("[key]");

  // value layout
  value_label_ = new QLabel;
  value_edit_ = new QLineEdit;
  value_edit_->setPlaceholderText("[value]");
  VERIFY(connect(value_edit_, &QLineEdit::textChanged, this, &KeyEditWidget::keyChanged));

  json_value_edit_ = new FastoViewer;
  VERIFY(connect(json_value_edit_, &FastoViewer::textChanged, this, &KeyEditWidget::keyChanged));

  bool_value_edit_ = new QComboBox;
  bool_value_edit_->addItem("true");
  bool_value_edit_->addItem("false");
  VERIFY(connect(bool_value_edit_, &QComboBox::currentTextChanged, this, &KeyEditWidget::keyChanged));

  value_list_edit_ = new ListTypeWidget;
  // value_list_edit_->horizontalHeader()->hide();
  // value_list_edit_->verticalHeader()->hide();

  VERIFY(connect(value_list_edit_, &ListTypeWidget::dataChangedSignal, this, &KeyEditWidget::keyChanged));
  value_table_edit_ = new HashTypeWidget;
  // value_table_edit_->horizontalHeader()->hide();
  // value_table_edit_->verticalHeader()->hide();

  VERIFY(connect(value_table_edit_, &HashTypeWidget::dataChangedSignal, this, &KeyEditWidget::keyChanged));

  stream_table_edit_ = new StreamTypeWidget;
  VERIFY(connect(stream_table_edit_, &StreamTypeWidget::dataChangedSignal, this, &KeyEditWidget::keyChanged));

  QGridLayout* main_layout = new QGridLayout;
  main_layout->addWidget(type_label_, 0, 0);
  main_layout->addWidget(types_combo_box_, 0, 1);
  main_layout->addWidget(key_label_, 1, 0);
  main_layout->addWidget(key_edit_, 1, 1);
  main_layout->addWidget(value_label_, 2, 0);
  main_layout->addWidget(value_edit_, 2, 1);
  main_layout->addWidget(json_value_edit_, 2, 1);
  main_layout->addWidget(bool_value_edit_, 2, 1);
  main_layout->addWidget(value_list_edit_, 2, 1);
  main_layout->addWidget(value_table_edit_, 2, 1);
  main_layout->addWidget(stream_table_edit_, 2, 1);
  setLayout(main_layout);

  // sync
  changeType(0);
  retranslateUi();
}

void KeyEditWidget::initialize(const core::NDbKValue& key) {
  core::NValue val = key.GetValue();
  CHECK(val) << "Value of key must be inited!";
  common::Value::Type current_type = key.GetType();

  int current_index = -1;
  for (int i = 0; i < types_combo_box_->count(); ++i) {
    QVariant cur = types_combo_box_->itemData(i);
    common::Value::Type type = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(cur));
    if (current_type == type) {
      current_index = static_cast<int>(i);
      break;
    }
  }

  CHECK_NE(current_index, -1) << "Type should be in availible_types array!";

  // sync keyname box
  QString qkey;
  const auto nkey = key.GetKey();
  const auto raw_key = nkey.GetKey();
  if (common::ConvertFromBytes(raw_key.GetForCommandLine(), &qkey)) {
    key_edit_->setText(qkey);
  }

  types_combo_box_->setCurrentIndex(current_index);
  syncControls(val);
}

KeyEditWidget::~KeyEditWidget() {}

void KeyEditWidget::setEnableKeyEdit(bool key_edit) {
  key_edit_->setEnabled(key_edit);
}

common::Value* KeyEditWidget::createItem() const {
  int index = types_combo_box_->currentIndex();
  QVariant var = types_combo_box_->itemData(index);
  common::Value::Type type = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));
  if (type == common::Value::TYPE_ARRAY) {
    return value_list_edit_->arrayValue();
  } else if (type == common::Value::TYPE_SET) {
    return value_list_edit_->setValue();
  } else if (type == common::Value::TYPE_ZSET) {
    return value_table_edit_->zsetValue();
  } else if (type == common::Value::TYPE_HASH) {
    return value_table_edit_->hashValue();
  } else if (type == core::StreamValue::TYPE_STREAM) {
    return stream_table_edit_->streamValue();
  } else if (type == core::JsonValue::TYPE_JSON || type == common::Value::TYPE_STRING) {
    const auto json_or_text_str = json_value_edit_->text();
    if (type == common::Value::TYPE_STRING) {
      if (json_or_text_str.empty()) {
        return nullptr;
      }
      return common::Value::CreateStringValue(json_or_text_str);
    }

    if (!core::JsonValue::IsValidJson(json_or_text_str)) {
      return nullptr;
    }
    return new core::JsonValue(json_or_text_str);
  } else if (type == common::Value::TYPE_BOOLEAN) {
    int index = bool_value_edit_->currentIndex();
    return common::Value::CreateBooleanValue(index == 0);
  }

  const std::string text_str = common::ConvertToString(value_edit_->text());
  if (text_str.empty()) {
    return nullptr;
  }

  if (type == common::Value::TYPE_INTEGER) {
    int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateIntegerValue(res);
  } else if (type == common::Value::TYPE_UINTEGER) {
    unsigned int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to unsigned int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateUIntegerValue(res);
  } else if (type == common::Value::TYPE_LONG_INTEGER) {
    int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateLongIntegerValue(res);
  } else if (type == common::Value::TYPE_ULONG_INTEGER) {
    unsigned int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to unsigned int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateULongIntegerValue(res);
  } else if (type == common::Value::TYPE_LONG_LONG_INTEGER) {
    int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateLongLongIntegerValue(res);
  } else if (type == common::Value::TYPE_ULONG_LONG_INTEGER) {
    unsigned int res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to unsigned int failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateULongLongIntegerValue(res);
  } else if (type == common::Value::TYPE_DOUBLE) {
    double res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to double failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateDoubleValue(res);
  }

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}

void KeyEditWidget::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  base_class::changeEvent(e);
}

void KeyEditWidget::changeType(int index) {
  QVariant var = types_combo_box_->itemData(index);
  common::Value::Type type = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));

  value_edit_->clear();
  json_value_edit_->clear();
  value_table_edit_->clear();
  stream_table_edit_->clear();
  value_list_edit_->clear();

  if (type == common::Value::TYPE_ARRAY || type == common::Value::TYPE_SET) {
    if (type == common::Value::TYPE_ARRAY) {
      value_list_edit_->setCurrentMode(ListTypeWidget::kArray);
    } else if (type == common::Value::TYPE_SET) {
      value_list_edit_->setCurrentMode(ListTypeWidget::kSet);
    } else {
      NOTREACHED();
    }
    value_list_edit_->setVisible(true);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(false);
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(false);
  } else if (type == common::Value::TYPE_ZSET || type == common::Value::TYPE_HASH) {
    if (type == common::Value::TYPE_HASH) {
      value_table_edit_->setCurrentMode(HashTypeWidget::kHash);
    } else if (type == common::Value::TYPE_ZSET) {
      value_table_edit_->setCurrentMode(HashTypeWidget::kZset);
    } else {
      NOTREACHED();
    }
    value_table_edit_->setVisible(true);
    stream_table_edit_->setVisible(false);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(false);
    value_list_edit_->setVisible(false);
  } else if (type == common::Value::TYPE_BOOLEAN) {
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(false);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(true);
    value_list_edit_->setVisible(false);
  } else if (type == core::StreamValue::TYPE_STREAM) {
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(true);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(false);
    value_list_edit_->setVisible(false);
  } else if (type == core::JsonValue::TYPE_JSON || type == common::Value::TYPE_STRING) {
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(false);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(true);
    bool_value_edit_->setVisible(false);
    value_list_edit_->setVisible(false);
  } else {
    value_edit_->setVisible(true);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(false);
    value_list_edit_->setVisible(false);
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(false);
    if (type == common::Value::TYPE_INTEGER || type == common::Value::TYPE_UINTEGER) {
      value_edit_->setValidator(new QIntValidator(this));
    } else if (type == common::Value::TYPE_LONG_INTEGER || type == common::Value::TYPE_ULONG_INTEGER) {
      value_edit_->setValidator(new QIntValidator(this));
    } else if (type == common::Value::TYPE_LONG_LONG_INTEGER || type == common::Value::TYPE_ULONG_LONG_INTEGER) {
      value_edit_->setValidator(new QIntValidator(this));
    } else if (type == common::Value::TYPE_DOUBLE) {
      value_edit_->setValidator(new QDoubleValidator(this));
    } else {
      QRegExp rx(".*");
      value_edit_->setValidator(new QRegExpValidator(rx, this));
    }
  }

  emit typeChanged(type);
}

void KeyEditWidget::syncControls(const core::NValue& item) {
  if (!item) {
    return;
  }

  value_edit_->clear();
  json_value_edit_->clear();
  value_table_edit_->clear();
  stream_table_edit_->clear();
  value_list_edit_->clear();

  common::Value::Type type = item->GetType();
  if (type == common::Value::TYPE_ARRAY) {
    common::ArrayValue* arr = nullptr;
    if (item->GetAsList(&arr)) {
      for (auto it = arr->begin(); it != arr->end(); ++it) {
        const auto val = core::ConvertValue(*it, DEFAULT_DELIMITER);
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromBytes(val, &qval)) {
          value_list_edit_->insertRow(qval);
        }
      }
    }
  } else if (type == common::Value::TYPE_SET) {
    common::SetValue* set = nullptr;
    if (item->GetAsSet(&set)) {
      for (auto it = set->begin(); it != set->end(); ++it) {
        const auto val = core::ConvertValue(*it, DEFAULT_DELIMITER);
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromBytes(val, &qval)) {
          value_list_edit_->insertRow(qval);
        }
      }
    }
  } else if (type == common::Value::TYPE_ZSET) {
    common::ZSetValue* zset = nullptr;
    if (item->GetAsZSet(&zset)) {
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        const auto key_str = core::ConvertValue(key, DEFAULT_DELIMITER);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        const auto value_str = core::ConvertValue(value, DEFAULT_DELIMITER);
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromBytes(key_str, &ftext) && common::ConvertFromBytes(value_str, &stext)) {
          value_table_edit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (type == common::Value::TYPE_HASH) {
    common::HashValue* hash = nullptr;
    if (item->GetAsHash(&hash)) {
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        const auto key_str = core::ConvertValue(key, DEFAULT_DELIMITER);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        const auto value_str = core::ConvertValue(value, DEFAULT_DELIMITER);
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromBytes(key_str, &ftext) && common::ConvertFromBytes(value_str, &stext)) {
          value_table_edit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (type == core::StreamValue::TYPE_STREAM) {
    core::StreamValue* stream = static_cast<core::StreamValue*>(item.get());
    auto entr = stream->GetStreams();
    for (size_t i = 0; i != entr.size(); ++i) {
      auto ent = entr[i];
      stream_table_edit_->insertStream(ent);
    }
  } else if (type == core::JsonValue::TYPE_JSON || type == common::Value::TYPE_STRING) {
    common::Value::string_t text;
    if (item->GetAsString(&text)) {
      if (type == core::JsonValue::TYPE_JSON) {
        json_value_edit_->setView(JSON_VIEW);
        json_value_edit_->setViewChangeEnabled(false);
      } else {
        json_value_edit_->setView(RAW_VIEW);
        json_value_edit_->setViewChangeEnabled(true);
      }
      json_value_edit_->setText(text);
    }
  } else if (type == common::Value::TYPE_BOOLEAN) {
    bool val;
    if (item->GetAsBoolean(&val)) {
      bool_value_edit_->setCurrentIndex(val ? 0 : 1);
    }
  } else if (type == common::Value::TYPE_INTEGER || type == common::Value::TYPE_UINTEGER) {
    int val;
    if (item->GetAsInteger(&val)) {
      value_edit_->setText(QString::number(val));
    }
  } else if (type == common::Value::TYPE_LONG_INTEGER || type == common::Value::TYPE_ULONG_INTEGER) {
    long val;
    if (item->GetAsLongInteger(&val)) {
      value_edit_->setText(QString::number(val));
    }
  } else if (type == common::Value::TYPE_LONG_LONG_INTEGER || type == common::Value::TYPE_ULONG_LONG_INTEGER) {
    long long val;
    if (item->GetAsLongLongInteger(&val)) {
      value_edit_->setText(QString::number(val));
    }
  } else {
    common::Value::string_t text;
    if (item->GetAsString(&text)) {
      QString qval;
      if (common::ConvertFromBytes(text, &qval)) {
        value_edit_->setText(qval);
      }
    }
  }
}

bool KeyEditWidget::getKey(core::NDbKValue* key) const {
  if (!key) {
    return false;
  }

  const QString key_name = key_edit_->text();
  if (key_name.isEmpty()) {
    return false;
  }

  common::Value* obj = createItem();
  if (!obj) {
    return false;
  }

  const core::nkey_t ks(common::ConvertToCharBytes(key_name));
  *key = core::NDbKValue(core::NKey(ks), core::NValue(obj));
  return true;
}

void KeyEditWidget::retranslateUi() {
  value_label_->setText(translations::trValue + ":");
  key_label_->setText(translations::trKey + ":");
  type_label_->setText(translations::trType + ":");
}

}  // namespace gui
}  // namespace fastonosql
