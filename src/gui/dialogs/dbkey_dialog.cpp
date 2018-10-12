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
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#include <fastonosql/core/db_traits.h>
#include <fastonosql/core/value.h>

#include "gui/gui_factory.h"  // for GuiFactory

#include "gui/widgets/key_edit_widget.h"

#include "translations/global.h"

namespace {
const QString trInput = QObject::tr("Key/Value input");
const QSize kPrefListSize = QSize(600, 300);
const QSize kPrefHashSize = QSize(600, 300);
const QSize kPrefStreamSize = QSize(600, 300);
const QSize kPrefJsonSize = QSize(600, 300);
}  // namespace

namespace fastonosql {
namespace gui {

const QSize DbKeyDialog::min_dialog_size = QSize(360, 240);

DbKeyDialog::DbKeyDialog(const QString& title,
                         core::ConnectionType type,
                         const core::NDbKValue& key,
                         bool is_edit,
                         QWidget* parent)
    : base_class(parent), key_(key) {
  setWindowIcon(GuiFactory::GetInstance().icon(type));
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  std::vector<common::Value::Type> types = core::GetSupportedValueTypes(type);
  general_box_ = new KeyEditWidget(types, this);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(general_box_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DbKeyDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &DbKeyDialog::reject));
  layout->addWidget(buttonBox);

  setMinimumSize(min_dialog_size);

  VERIFY(connect(general_box_, &KeyEditWidget::typeChanged, this, &DbKeyDialog::changeType));
  general_box_->initialize(key_);
  general_box_->setEnableKeyEdit(!is_edit);

  setLayout(layout);
  retranslateUi();
}

core::NDbKValue DbKeyDialog::key() const {
  return key_;
}

void DbKeyDialog::accept() {
  if (!validateAndApply()) {
    QMessageBox::warning(this, translations::trInvalidInput, translations::trInvalidInput + "!");
    return;
  }

  QDialog::accept();
}

void DbKeyDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  base_class::changeEvent(e);
}

bool DbKeyDialog::validateAndApply() {
  return general_box_->getKey(&key_);
}

void DbKeyDialog::retranslateUi() {
  general_box_->setTitle(trInput);
}

void DbKeyDialog::changeType(common::Value::Type type) {
  if (type == common::Value::TYPE_ARRAY || type == common::Value::TYPE_SET) {
    resize(kPrefListSize);
  } else if (type == common::Value::TYPE_ZSET || type == common::Value::TYPE_HASH) {
    resize(kPrefHashSize);
  } else if (type == core::StreamValue::TYPE_STREAM) {
    resize(kPrefStreamSize);
  } else if (type == core::JsonValue::TYPE_JSON || type == core::JsonValue::TYPE_STRING) {
    resize(kPrefJsonSize);
  } else {
    resize(min_dialog_size);
  }
}

}  // namespace gui
}  // namespace fastonosql
