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

#include "gui/dialogs/dbkey_dialog.h"

#include <stddef.h>  // for size_t

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/macros.h>             // for VERIFY, CHECK, NOTREACHED
#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/utils_qt.h>
#include <common/value.h>  // for Value, Value::Type, etc

#include "core/db_traits.h"
#include "core/global.h"

#include "gui/widgets/hash_type_widget.h"
#include "gui/widgets/list_type_widget.h"

#include "gui/dialogs/input_dialog.h"  // for InputDialog, etc
#include "gui/gui_factory.h"           // for GuiFactory
#include "gui/hash_table_model.h"

#include "translations/global.h"  // for trAddItem, trRemoveItem, etc

namespace {
const QString trType = QObject::tr("Type:");
const QString trKey = QObject::tr("Key:");
const QString trValue = QObject::tr("Value:");
const QString trInput = QObject::tr("Key/Value input");
}  // namespace

namespace fastonosql {
namespace gui {

DbKeyDialog::DbKeyDialog(const QString& title, core::connectionTypes type, const core::NDbKValue& key, QWidget* parent)
    : QDialog(parent), type_(type), key_(key) {
  bool is_edit = !key.Equals(core::NDbKValue());
  setWindowIcon(GuiFactory::Instance().icon(type));
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)
  QGridLayout* kvLayout = new QGridLayout;
  kvLayout->addWidget(new QLabel(trType), 0, 0);
  typesCombo_ = new QComboBox;
  std::vector<common::Value::Type> types = SupportedTypesFromType(type);
  common::Value::Type kt = common::Value::TYPE_STRING;
  if (is_edit) {
    kt = key_.Type();
  }
  int current_index = 0;
  for (size_t i = 0; i < types.size(); ++i) {
    common::Value::Type t = types[i];
    if (kt == t) {
      current_index = i;
    }
    QString type;
    if (common::ConvertFromString(common::Value::GetTypeName(t), &type)) {
      typesCombo_->addItem(GuiFactory::Instance().icon(t), type, t);
    }
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(typesCombo_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &DbKeyDialog::typeChanged));
  kvLayout->addWidget(typesCombo_, 0, 1);

  // key layout
  kvLayout->addWidget(new QLabel(trKey), 1, 0);
  keyEdit_ = new QLineEdit;
  keyEdit_->setPlaceholderText("[key]");
  kvLayout->addWidget(keyEdit_, 1, 1);

  // value layout
  kvLayout->addWidget(new QLabel(trValue), 2, 0);
  valueEdit_ = new QLineEdit;
  valueEdit_->setPlaceholderText("[value]");
  kvLayout->addWidget(valueEdit_, 2, 1);
  valueEdit_->setVisible(true);

  boolValueEdit_ = new QComboBox;
  boolValueEdit_->addItem("true");
  boolValueEdit_->addItem("false");
  kvLayout->addWidget(boolValueEdit_, 2, 1);
  boolValueEdit_->setVisible(false);

  valueListEdit_ = new ListTypeWidget;
  valueListEdit_->horizontalHeader()->hide();
  valueListEdit_->verticalHeader()->hide();
  kvLayout->addWidget(valueListEdit_, 2, 1);
  valueListEdit_->setVisible(false);

  valueTableEdit_ = new HashTypeWidget;
  valueTableEdit_->horizontalHeader()->hide();
  valueTableEdit_->verticalHeader()->hide();
  kvLayout->addWidget(valueTableEdit_, 2, 1);
  valueTableEdit_->setVisible(false);

  generalBox_ = new QGroupBox(this);
  generalBox_->setLayout(kvLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(generalBox_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DbKeyDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &DbKeyDialog::reject));
  layout->addWidget(buttonBox);

  if (is_edit) {
    QString qkey;
    if (common::ConvertFromString(key_.KeyString(), &qkey)) {
      keyEdit_->setText(qkey);
    }
    keyEdit_->setEnabled(false);
  }
  typesCombo_->setCurrentIndex(current_index);
  core::NValue val = key_.Value();
  syncControls(val.get());

  setMinimumSize(QSize(min_width, min_height));
  setLayout(layout);
  retranslateUi();
}

core::NDbKValue DbKeyDialog::key() const {
  return key_;
}

void DbKeyDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void DbKeyDialog::typeChanged(int index) {
  QVariant var = typesCombo_->itemData(index);
  common::Value::Type type = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));

  valueEdit_->clear();
  valueTableEdit_->clear();
  valueListEdit_->clear();

  if (type == common::Value::TYPE_ARRAY || type == common::Value::TYPE_SET) {
    valueListEdit_->setVisible(true);
    valueEdit_->setVisible(false);
    boolValueEdit_->setVisible(false);
    valueTableEdit_->setVisible(false);
  } else if (type == common::Value::TYPE_ZSET || type == common::Value::TYPE_HASH) {
    valueTableEdit_->setVisible(true);
    valueEdit_->setVisible(false);
    boolValueEdit_->setVisible(false);
    valueListEdit_->setVisible(false);
  } else {
    valueEdit_->setVisible(true);
    boolValueEdit_->setVisible(false);
    valueListEdit_->setVisible(false);
    valueTableEdit_->setVisible(false);
    if (type == common::Value::TYPE_INTEGER || type == common::Value::TYPE_UINTEGER) {
      valueEdit_->setValidator(new QIntValidator(this));
    } else if (type == common::Value::TYPE_BOOLEAN) {
      boolValueEdit_->setVisible(true);
      valueEdit_->setVisible(false);
    } else if (type == common::Value::TYPE_DOUBLE) {
      valueEdit_->setValidator(new QDoubleValidator(this));
    } else {
      QRegExp rx(".*");
      valueEdit_->setValidator(new QRegExpValidator(rx, this));
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
        std::string val = common::ConvertToString(*it, "");
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromString(val, &qval)) {
          valueListEdit_->insertRow(qval);
        }
      }
    }
  } else if (t == common::Value::TYPE_SET) {
    common::SetValue* set = nullptr;
    if (item->GetAsSet(&set)) {
      for (auto it = set->begin(); it != set->end(); ++it) {
        std::string val = common::ConvertToString(*it, "");
        if (val.empty()) {
          continue;
        }

        QString qval;
        if (common::ConvertFromString(val, &qval)) {
          valueListEdit_->insertRow(qval);
        }
      }
    }
  } else if (t == common::Value::TYPE_ZSET) {
    common::ZSetValue* zset = nullptr;
    if (item->GetAsZSet(&zset)) {
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        std::string key_str = common::ConvertToString(key, "");
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        std::string value_str = common::ConvertToString(value, "");
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromString(key_str, &ftext) && common::ConvertFromString(value_str, &stext)) {
          valueTableEdit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (t == common::Value::TYPE_HASH) {
    common::HashValue* hash = nullptr;
    if (item->GetAsHash(&hash)) {
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        std::string key_str = common::ConvertToString(key, "");
        if (key_str.empty()) {
          continue;
        }

        common::Value* value = element.second;
        std::string value_str = common::ConvertToString(value, "");
        if (value_str.empty()) {
          continue;
        }

        QString ftext;
        QString stext;
        if (common::ConvertFromString(key_str, &ftext) && common::ConvertFromString(value_str, &stext)) {
          valueTableEdit_->insertRow(ftext, stext);
        }
      }
    }
  } else if (t == common::Value::TYPE_BOOLEAN) {
    bool val;
    if (item->GetAsBoolean(&val)) {
      boolValueEdit_->setCurrentIndex(val ? 0 : 1);
    }
  } else {
    std::string text;
    if (item->GetAsString(&text)) {
      QString qval;
      if (common::ConvertFromString(text, &qval)) {
        valueEdit_->setText(qval);
      }
    }
  }
}

bool DbKeyDialog::validateAndApply() {
  if (keyEdit_->text().isEmpty()) {
    return false;
  }

  common::Value* obj = item();
  if (!obj) {
    return false;
  }

  core::NKey key(common::ConvertToString(keyEdit_->text()));
  key_ = core::NDbKValue(key, core::NValue(obj));
  return true;
}

void DbKeyDialog::retranslateUi() {
  generalBox_->setTitle(trInput);
}

common::Value* DbKeyDialog::item() const {
  int index = typesCombo_->currentIndex();
  QVariant var = typesCombo_->itemData(index);
  common::Value::Type t = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));
  if (t == common::Value::TYPE_ARRAY) {
    return valueListEdit_->arrayValue();
  } else if (t == common::Value::TYPE_SET) {
    return valueListEdit_->setValue();
  } else if (t == common::Value::TYPE_ZSET) {
    return valueTableEdit_->zsetValue();
  } else if (t == common::Value::TYPE_HASH) {
    return valueTableEdit_->hashValue();
  } else if (t == common::Value::TYPE_BOOLEAN) {
    int index = boolValueEdit_->currentIndex();
    if (index == -1) {
      return nullptr;
    }

    return common::Value::CreateBooleanValue(index == 0);
  } else {
    QString text = valueEdit_->text();
    if (text.isEmpty()) {
      return nullptr;
    }

    return common::Value::CreateStringValue(common::ConvertToString(text));
  }
}

}  // namespace gui
}  // namespace fastonosql
