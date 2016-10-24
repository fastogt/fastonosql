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
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/macros.h>             // for VERIFY, CHECK, NOTREACHED
#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/value.h>              // for Value, Value::Type, etc

#include "core/db_traits.h"

#include "gui/dialogs/input_dialog.h"  // for InputDialog, etc
#include "gui/gui_factory.h"           // for GuiFactory

#include "translations/global.h"  // for trAddItem, trRemoveItem, etc

namespace {
const QString trType = QObject::tr("Type:");
const QString trKey = QObject::tr("Key:");
const QString trValue = QObject::tr("Value:");
const QString trInput = QObject::tr("Key/Value input");
}

namespace fastonosql {
namespace gui {
DbKeyDialog::DbKeyDialog(const QString& title,
                         core::connectionTypes type,
                         core::NDbKValue key,
                         QWidget* parent)
    : QDialog(parent), type_(type), key_(key) {
  bool is_edit = !key.equals(core::NDbKValue());
  setWindowIcon(GuiFactory::instance().icon(type));
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)
  QGridLayout* kvLayout = new QGridLayout;
  kvLayout->addWidget(new QLabel(trType), 0, 0);
  typesCombo_ = new QComboBox;
  std::vector<common::Value::Type> types = SupportedTypesFromType(type);
  common::Value::Type kt = common::Value::TYPE_STRING;
  if (is_edit) {
    kt = key_.type();
  }
  int current_index = 0;
  for (size_t i = 0; i < types.size(); ++i) {
    common::Value::Type t = types[i];
    if (kt == t) {
      current_index = i;
    }
    QString type = common::ConvertFromString<QString>(common::Value::toString(t));
    typesCombo_->addItem(GuiFactory::instance().icon(t), type, t);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(typesCombo_, static_cast<ind>(&QComboBox::currentIndexChanged), this,
                 &DbKeyDialog::typeChanged));
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

  valueListEdit_ = new QListWidget;
  valueListEdit_->setContextMenuPolicy(Qt::ActionsContextMenu);
  valueListEdit_->setSelectionMode(QAbstractItemView::SingleSelection);
  valueListEdit_->setSelectionBehavior(QAbstractItemView::SelectRows);

  QAction* addItem = new QAction(translations::trAddItem, this);
  VERIFY(connect(addItem, &QAction::triggered, this, &DbKeyDialog::addItem));
  valueListEdit_->addAction(addItem);

  QAction* removeItem = new QAction(translations::trRemoveItem, this);
  VERIFY(connect(removeItem, &QAction::triggered, this, &DbKeyDialog::removeItem));
  valueListEdit_->addAction(removeItem);

  kvLayout->addWidget(valueListEdit_, 2, 1);
  valueListEdit_->setVisible(false);

  valueTableEdit_ = new QTableWidget(0, 2);
  valueTableEdit_->setContextMenuPolicy(Qt::ActionsContextMenu);
  valueTableEdit_->setSelectionBehavior(QAbstractItemView::SelectRows);
  valueTableEdit_->verticalHeader()->hide();
  valueTableEdit_->horizontalHeader()->hide();

  valueTableEdit_->addAction(addItem);
  valueTableEdit_->addAction(removeItem);

  kvLayout->addWidget(valueTableEdit_, 2, 1);
  valueTableEdit_->setVisible(false);

  addItemButton_ = new QPushButton(translations::trAddItem);
  VERIFY(connect(addItemButton_, &QPushButton::clicked, this, &DbKeyDialog::addItem));
  kvLayout->addWidget(addItemButton_, 3, 0);
  addItemButton_->setVisible(false);

  removeItemButton_ = new QPushButton(translations::trRemoveItem);
  VERIFY(connect(removeItemButton_, &QPushButton::clicked, this, &DbKeyDialog::removeItem));
  kvLayout->addWidget(removeItemButton_, 3, 1);
  removeItemButton_->setVisible(false);

  generalBox_ = new QGroupBox(this);
  generalBox_->setLayout(kvLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(generalBox_);

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DbKeyDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &DbKeyDialog::reject));
  layout->addWidget(buttonBox);

  if (is_edit) {
    keyEdit_->setText(common::ConvertFromString<QString>(key_.keyString()));
    keyEdit_->setEnabled(false);
  }
  typesCombo_->setCurrentIndex(current_index);
  core::NValue val = key_.value();
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
    addItemButton_->setVisible(true);
    removeItemButton_->setVisible(true);
  } else if (type == common::Value::TYPE_ZSET || type == common::Value::TYPE_HASH) {
    valueTableEdit_->setVisible(true);
    valueEdit_->setVisible(false);
    boolValueEdit_->setVisible(false);
    valueListEdit_->setVisible(false);
    addItemButton_->setVisible(true);
    removeItemButton_->setVisible(true);
  } else {
    valueEdit_->setVisible(true);
    boolValueEdit_->setVisible(false);
    valueListEdit_->setVisible(false);
    valueTableEdit_->setVisible(false);
    addItemButton_->setVisible(false);
    removeItemButton_->setVisible(false);
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

void DbKeyDialog::addItem() {
  int index = typesCombo_->currentIndex();
  QVariant var = typesCombo_->itemData(index);
  common::Value::Type t = static_cast<common::Value::Type>(qvariant_cast<unsigned char>(var));

  if (valueListEdit_->isVisible()) {  // array
    CHECK(t == common::Value::TYPE_SET || t == common::Value::TYPE_ARRAY);
    InputDialog diag(this, translations::trAddItem, InputDialog::SingleLine, translations::trValue);
    if (t == common::Value::TYPE_SET) {
      diag.setFirstPlaceholderText("[member]");
    } else if (t == common::Value::TYPE_ARRAY) {
      diag.setFirstPlaceholderText("[value]");
    }
    int result = diag.exec();
    if (result != QDialog::Accepted) {
      return;
    }

    QString text = diag.firstText();
    if (!text.isEmpty()) {
      QListWidgetItem* nitem = new QListWidgetItem(text, valueListEdit_);
      nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
      valueListEdit_->addItem(nitem);
    }
  } else if (valueTableEdit_->isVisible()) {
    CHECK(t == common::Value::TYPE_HASH || t == common::Value::TYPE_ZSET);
    common::scoped_ptr<InputDialog> diag;
    if (t == common::Value::TYPE_HASH) {
      diag.reset(new InputDialog(this, translations::trAddItem, InputDialog::DoubleLine,
                                 translations::trField, translations::trValue));
      diag->setFirstPlaceholderText("[field]");
      diag->setSecondPlaceholderText("[value]");
    } else if (t == common::Value::TYPE_ZSET) {
      diag.reset(new InputDialog(this, translations::trAddItem, InputDialog::DoubleLine,
                                 translations::trScore, translations::trMember));
      diag->setFirstPlaceholderText("[score]");
      diag->setSecondPlaceholderText("[member]");
    }

    int result = diag->exec();
    if (result != QDialog::Accepted) {
      return;
    }

    QString ftext = diag->firstText();
    QString stext = diag->secondText();

    if (!ftext.isEmpty() && !stext.isEmpty()) {
      QTableWidgetItem* fitem = new QTableWidgetItem(ftext);
      fitem->setFlags(fitem->flags() | Qt::ItemIsEditable);

      QTableWidgetItem* sitem = new QTableWidgetItem(stext);
      sitem->setFlags(sitem->flags() | Qt::ItemIsEditable);

      valueTableEdit_->insertRow(0);
      valueTableEdit_->setItem(0, 0, fitem);
      valueTableEdit_->setItem(0, 1, sitem);
    }
  } else if (valueEdit_->isVisible()) {
    CHECK(t == common::Value::TYPE_STRING || t == common::Value::TYPE_DOUBLE ||
          t == common::Value::TYPE_INTEGER || t == common::Value::TYPE_UINTEGER);
  } else if (boolValueEdit_->isVisible()) {
    CHECK(t == common::Value::TYPE_BOOLEAN);
  } else {
    NOTREACHED();
  }
}

void DbKeyDialog::removeItem() {
  if (valueListEdit_->isVisible()) {
    QListWidgetItem* ritem = valueListEdit_->currentItem();
    delete ritem;
  } else if (valueTableEdit_->isVisible()) {
    int row = valueTableEdit_->currentRow();
    valueTableEdit_->removeRow(row);
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

  common::Value::Type t = item->type();
  if (t == common::Value::TYPE_ARRAY) {
    common::ArrayValue* arr = NULL;
    if (item->getAsList(&arr)) {
      for (auto it = arr->begin(); it != arr->end(); ++it) {
        std::string val = (*it)->toString();
        if (val.empty()) {
          continue;
        }

        QListWidgetItem* nitem =
            new QListWidgetItem(common::ConvertFromString<QString>(val), valueListEdit_);
        nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
        valueListEdit_->addItem(nitem);
      }
    }
  } else if (t == common::Value::TYPE_SET) {
    common::SetValue* set = NULL;
    if (item->getAsSet(&set)) {
      for (auto it = set->begin(); it != set->end(); ++it) {
        std::string val = (*it)->toString();
        if (val.empty()) {
          continue;
        }

        QListWidgetItem* nitem =
            new QListWidgetItem(common::ConvertFromString<QString>(val), valueListEdit_);
        nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
        valueListEdit_->addItem(nitem);
      }
    }
  } else if (t == common::Value::TYPE_ZSET) {
    common::ZSetValue* zset = NULL;
    if (item->getAsZSet(&zset)) {
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        common::Value* value = element.second;
        QString ftext = common::ConvertFromString<QString>(key->toString());
        QString stext = common::ConvertFromString<QString>(value->toString());

        if (!ftext.isEmpty() && !stext.isEmpty()) {
          QTableWidgetItem* fitem = new QTableWidgetItem(ftext);
          fitem->setFlags(fitem->flags() | Qt::ItemIsEditable);

          QTableWidgetItem* sitem = new QTableWidgetItem(stext);
          sitem->setFlags(sitem->flags() | Qt::ItemIsEditable);

          valueTableEdit_->insertRow(0);
          valueTableEdit_->setItem(0, 0, fitem);
          valueTableEdit_->setItem(0, 1, sitem);
        }
      }
    }
  } else if (t == common::Value::TYPE_HASH) {
    common::HashValue* hash = NULL;
    if (item->getAsHash(&hash)) {
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto element = (*it);
        common::Value* key = element.first;
        common::Value* value = element.second;
        QString ftext = common::ConvertFromString<QString>(key->toString());
        QString stext = common::ConvertFromString<QString>(value->toString());

        if (!ftext.isEmpty() && !stext.isEmpty()) {
          QTableWidgetItem* fitem = new QTableWidgetItem(ftext);
          fitem->setFlags(fitem->flags() | Qt::ItemIsEditable);

          QTableWidgetItem* sitem = new QTableWidgetItem(stext);
          sitem->setFlags(sitem->flags() | Qt::ItemIsEditable);

          valueTableEdit_->insertRow(0);
          valueTableEdit_->setItem(0, 0, fitem);
          valueTableEdit_->setItem(0, 1, sitem);
        }
      }
    }
  } else if (t == common::Value::TYPE_BOOLEAN) {
    bool val;
    if (item->getAsBoolean(&val)) {
      boolValueEdit_->setCurrentIndex(val ? 0 : 1);
    }
  } else {
    std::string text;
    if (item->getAsString(&text)) {
      valueEdit_->setText(common::ConvertFromString<QString>(text));
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
    if (valueListEdit_->count() == 0) {
      return nullptr;
    }

    common::ArrayValue* ar = common::Value::createArrayValue();
    for (int i = 0; i < valueListEdit_->count(); ++i) {
      std::string val = common::ConvertToString(valueListEdit_->item(i)->text());
      ar->appendString(val);
    }

    return ar;
  } else if (t == common::Value::TYPE_SET) {
    if (valueListEdit_->count() == 0) {
      return nullptr;
    }

    common::SetValue* ar = common::Value::createSetValue();
    for (int i = 0; i < valueListEdit_->count(); ++i) {
      std::string val = common::ConvertToString(valueListEdit_->item(i)->text());
      ar->insert(val);
    }

    return ar;
  } else if (t == common::Value::TYPE_ZSET) {
    if (valueTableEdit_->rowCount() == 0) {
      return nullptr;
    }

    common::ZSetValue* ar = common::Value::createZSetValue();
    for (int i = 0; i < valueTableEdit_->rowCount(); ++i) {
      QTableWidgetItem* kitem = valueTableEdit_->item(i, 0);
      QTableWidgetItem* vitem = valueTableEdit_->item(i, 1);

      std::string key = common::ConvertToString(kitem->text());
      std::string val = common::ConvertToString(vitem->text());
      ar->insert(key, val);
    }

    return ar;
  } else if (t == common::Value::TYPE_HASH) {
    if (valueTableEdit_->rowCount() == 0) {
      return nullptr;
    }

    common::HashValue* ar = common::Value::createHashValue();
    for (int i = 0; i < valueTableEdit_->rowCount(); ++i) {
      QTableWidgetItem* kitem = valueTableEdit_->item(i, 0);
      QTableWidgetItem* vitem = valueTableEdit_->item(i, 1);

      std::string key = common::ConvertToString(kitem->text());
      std::string val = common::ConvertToString(vitem->text());
      ar->insert(key, val);
    }

    return ar;
  } else if (t == common::Value::TYPE_BOOLEAN) {
    int index = boolValueEdit_->currentIndex();
    if (index == -1) {
      return nullptr;
    }

    return common::Value::createBooleanValue(index == 0 ? true : false);
  } else {
    QString text = valueEdit_->text();
    if (text.isEmpty()) {
      return nullptr;
    }

    return common::Value::createStringValue(common::ConvertToString(text));
  }
}
}  // namespace gui
}  // namespace fastonosql
