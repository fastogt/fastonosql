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

#include "gui/dialogs/dbkey_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#include <Qsci/qscilexerjson.h>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>  // for ConvertToString

#include "core/db_traits.h"
#include "core/value.h"

#include "gui/widgets/hash_type_widget.h"
#include "gui/widgets/list_type_widget.h"
#include "gui/widgets/stream_type_widget.h"

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/hash_table_model.h"

#include "gui/editor/fasto_editor.h"

#include "translations/global.h"  // for trAddItem, trRemoveItem, etc

namespace {
const QString trInput = QObject::tr("Key/Value input");
}  // namespace

namespace fastonosql {
namespace gui {

DbKeyDialog::DbKeyDialog(const QString& title, core::connectionTypes type, const core::NDbKValue& key, QWidget* parent)
    : QDialog(parent), key_(key) {
  bool is_edit = !key.Equals(core::NDbKValue());
  setWindowIcon(GuiFactory::GetInstance().GetIcon(type));
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)
  QGridLayout* kvLayout = new QGridLayout;
  type_label_ = new QLabel;
  kvLayout->addWidget(type_label_, 0, 0);
  types_combo_box_ = new QComboBox;
  std::vector<common::Value::Type> types = core::GetSupportedValueTypes(type);
  common::Value::Type kt = common::Value::TYPE_STRING;
  if (is_edit) {
    kt = key_.GetType();
  }
  int current_index = 0;
  for (size_t i = 0; i < types.size(); ++i) {
    common::Value::Type t = types[i];
    if (kt == t) {
      current_index = static_cast<int>(i);
    }
    QString type = core::GetTypeName(t);
    types_combo_box_->addItem(GuiFactory::GetInstance().GetIcon(t), type, t);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(types_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &DbKeyDialog::typeChanged));
  kvLayout->addWidget(types_combo_box_, 0, 1);

  // key layout
  key_label_ = new QLabel;
  kvLayout->addWidget(key_label_, 1, 0);
  key_edit_ = new QLineEdit;
  key_edit_->setPlaceholderText("[key]");
  kvLayout->addWidget(key_edit_, 1, 1);

  // value layout
  value_label_ = new QLabel;
  kvLayout->addWidget(value_label_, 2, 0);
  value_edit_ = new QLineEdit;
  value_edit_->setPlaceholderText("[value]");
  kvLayout->addWidget(value_edit_, 2, 1);
  value_edit_->setVisible(true);

  json_value_edit_ = new FastoEditor;
  QsciLexerJSON* json_lexer = new QsciLexerJSON;
  json_value_edit_->setLexer(json_lexer);
  kvLayout->addWidget(json_value_edit_, 2, 1);
  json_value_edit_->setVisible(false);

  bool_value_edit_ = new QComboBox;
  bool_value_edit_->addItem("true");
  bool_value_edit_->addItem("false");
  kvLayout->addWidget(bool_value_edit_, 2, 1);
  bool_value_edit_->setVisible(false);

  value_list_edit_ = new ListTypeWidget;
  // value_list_edit_->horizontalHeader()->hide();
  // value_list_edit_->verticalHeader()->hide();
  kvLayout->addWidget(value_list_edit_, 2, 1);
  value_list_edit_->setVisible(false);

  value_table_edit_ = new HashTypeWidget;
  // value_table_edit_->horizontalHeader()->hide();
  // value_table_edit_->verticalHeader()->hide();
  kvLayout->addWidget(value_table_edit_, 2, 1);
  value_table_edit_->setVisible(false);

  stream_table_edit_ = new StreamTypeWidget;
  kvLayout->addWidget(stream_table_edit_, 2, 1);
  stream_table_edit_->setVisible(false);

  general_box_ = new QGroupBox(this);
  general_box_->setLayout(kvLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(general_box_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DbKeyDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &DbKeyDialog::reject));
  layout->addWidget(buttonBox);

  if (is_edit) {
    QString qkey;
    core::NKey key = key_.GetKey();
    core::key_t raw_key = key.GetKey();
    if (common::ConvertFromString(raw_key.GetHumanReadable(), &qkey)) {
      key_edit_->setText(qkey);
    }
    key_edit_->setEnabled(false);
  }
  types_combo_box_->setCurrentIndex(current_index);
  core::NValue val = key_.GetValue();
  syncControls(val.get());

  setMinimumSize(QSize(min_width, min_height));
  setLayout(layout);
  retranslateUi();
}

core::NDbKValue DbKeyDialog::GetKey() const {
  return key_;
}

void DbKeyDialog::accept() {
  if (!validateAndApply()) {
    QMessageBox::warning(this, translations::trInvalidInput, translations::trInvalidInput + "!");
    return;
  }

  QDialog::accept();
}

void DbKeyDialog::typeChanged(int index) {
  QVariant var = types_combo_box_->itemData(index);
  common::Value::Type type = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));

  value_edit_->clear();
  json_value_edit_->clear();
  value_table_edit_->clear();
  stream_table_edit_->clear();
  value_list_edit_->clear();

  if (type == common::Value::TYPE_ARRAY || type == common::Value::TYPE_SET) {
    value_list_edit_->setVisible(true);
    value_edit_->setVisible(false);
    json_value_edit_->setVisible(false);
    bool_value_edit_->setVisible(false);
    value_table_edit_->setVisible(false);
    stream_table_edit_->setVisible(false);
  } else if (type == common::Value::TYPE_ZSET || type == common::Value::TYPE_HASH) {
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
  } else if (type == core::JsonValue::TYPE_JSON) {
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
    } else if (type == common::Value::TYPE_DOUBLE) {
      value_edit_->setValidator(new QDoubleValidator(this));
    } else {
      QRegExp rx(".*");
      value_edit_->setValidator(new QRegExpValidator(rx, this));
    }
  }
}

void DbKeyDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void DbKeyDialog::syncControls(common::Value* item) {
  if (!item) {
    return;
  }

  common::Value::Type t = item->GetType();
  if (t == common::Value::TYPE_ARRAY) {
    common::ArrayValue* arr = nullptr;
    if (item->GetAsList(&arr)) {
      for (auto it = arr->begin(); it != arr->end(); ++it) {
        std::string val = core::ConvertValue(*it, DEFAULT_DELIMITER);
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromString(val, &qval)) {
          value_list_edit_->insertRow(qval);
        }
      }
    }
  } else if (t == common::Value::TYPE_SET) {
    common::SetValue* set = nullptr;
    if (item->GetAsSet(&set)) {
      for (auto it = set->begin(); it != set->end(); ++it) {
        std::string val = core::ConvertValue(*it, DEFAULT_DELIMITER);
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromString(val, &qval)) {
          value_list_edit_->insertRow(qval);
        }
      }
    }
  } else if (t == common::Value::TYPE_ZSET) {
    common::ZSetValue* zset = nullptr;
    if (item->GetAsZSet(&zset)) {
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        std::string key_str = core::ConvertValue(key, DEFAULT_DELIMITER);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        std::string value_str = core::ConvertValue(value, DEFAULT_DELIMITER);
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromString(key_str, &ftext) && common::ConvertFromString(value_str, &stext)) {
          value_table_edit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (t == common::Value::TYPE_HASH) {
    common::HashValue* hash = nullptr;
    if (item->GetAsHash(&hash)) {
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        std::string key_str = core::ConvertValue(key, DEFAULT_DELIMITER);
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        std::string value_str = core::ConvertValue(value, DEFAULT_DELIMITER);
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromString(key_str, &ftext) && common::ConvertFromString(value_str, &stext)) {
          value_table_edit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (t == core::StreamValue::TYPE_STREAM) {
    core::StreamValue* stream = static_cast<core::StreamValue*>(item);
    auto entr = stream->GetStreams();
    for (size_t i = 0; i != entr.size(); ++i) {
      auto ent = entr[i];
      stream_table_edit_->insertStream(ent);
    }
  } else if (t == core::JsonValue::TYPE_JSON) {
    std::string text;
    if (item->GetAsString(&text)) {
      QString qval;
      if (common::ConvertFromString(text, &qval)) {
        json_value_edit_->setText(qval);
      }
    }
  } else if (t == common::Value::TYPE_BOOLEAN) {
    bool val;
    if (item->GetAsBoolean(&val)) {
      bool_value_edit_->setCurrentIndex(val ? 0 : 1);
    }
  } else {
    std::string text;
    if (item->GetAsString(&text)) {
      QString qval;
      if (common::ConvertFromString(text, &qval)) {
        value_edit_->setText(qval);
      }
    }
  }
}

bool DbKeyDialog::validateAndApply() {
  if (key_edit_->text().isEmpty()) {
    return false;
  }

  common::Value* obj = item();
  if (!obj) {
    return false;
  }

  std::string key_str = common::ConvertToString(key_edit_->text());
  core::key_t ks(key_str);
  core::NKey key(ks);
  key_ = core::NDbKValue(key, core::NValue(obj));
  return true;
}

void DbKeyDialog::retranslateUi() {
  value_label_->setText(translations::trValue + ":");
  key_label_->setText(translations::trKey + ":");
  type_label_->setText(translations::trType + ":");
  general_box_->setTitle(trInput);
}

common::Value* DbKeyDialog::item() const {
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
    return stream_table_edit_->GetStreamValue();
  } else if (type == core::JsonValue::TYPE_JSON) {
    const std::string text_str = common::ConvertToString(json_value_edit_->text());
    if (!core::JsonValue::IsValidJson(text_str)) {
      return nullptr;
    }
    return new core::JsonValue(text_str);
  } else if (type == common::Value::TYPE_BOOLEAN) {
    int index = bool_value_edit_->currentIndex();
    return common::Value::CreateBooleanValue(index == 0);
  }

  const std::string text_str = common::ConvertToString(value_edit_->text());
  if (text_str.empty()) {
    DNOTREACHED() << "Invalid user input.";
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
  } else if (type == common::Value::TYPE_DOUBLE) {
    double res;
    bool is_ok = common::ConvertFromString(text_str, &res);
    if (!is_ok) {
      DNOTREACHED() << "Conversion to double failed, text: " << text_str;
      return nullptr;
    }
    return common::Value::CreateDoubleValue(res);
  }

  return common::Value::CreateStringValue(text_str);
}

}  // namespace gui
}  // namespace fastonosql
