/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/dialogs/load_contentdb_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include <common/macros.h>

#include <fastonosql/core/macros.h>

#include "translations/global.h"

namespace {
const QString trInvalidPattern = QObject::tr("Invalid pattern!");
const QString trKeysCount = QObject::tr("Keys count");
const QString trPattern = QObject::tr("Pattern");
const char* kDefaultPattern = ALL_KEYS_PATTERNS;
}  // namespace

namespace fastonosql {
namespace gui {

LoadContentDbDialog::LoadContentDbDialog(const QString& title, const QIcon& icon, QWidget* parent)
    : base_class(title, parent),
      keys_count_label_(nullptr),
      key_pattern_label_(nullptr),
      pattern_edit_(nullptr),
      count_spin_edit_(nullptr) {
  setWindowIcon(icon);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &LoadContentDbDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &LoadContentDbDialog::reject));

  QHBoxLayout* count_layout = new QHBoxLayout;
  keys_count_label_ = new QLabel;
  count_layout->addWidget(keys_count_label_);
  count_spin_edit_ = new QSpinBox;
  count_spin_edit_->setRange(min_key_on_page, max_key_on_page);
  count_spin_edit_->setSingleStep(step_keys_on_page);
  count_spin_edit_->setValue(defaults_key);
  count_layout->addWidget(count_spin_edit_);

  QHBoxLayout* pattern_layout = new QHBoxLayout;
  key_pattern_label_ = new QLabel;
  pattern_layout->addWidget(key_pattern_label_);
  pattern_edit_ = new QLineEdit;
  pattern_edit_->setFixedWidth(80);
  pattern_edit_->setText(kDefaultPattern);
  pattern_layout->addWidget(pattern_edit_);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(count_layout);
  main_layout->addLayout(pattern_layout);
  main_layout->addWidget(button_box);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);
}

int LoadContentDbDialog::count() const {
  return count_spin_edit_->value();
}

QString LoadContentDbDialog::pattern() const {
  return pattern_edit_->text();
}

void LoadContentDbDialog::accept() {
  const QString pattern = pattern_edit_->text();
  if (pattern.isEmpty()) {
    QMessageBox::warning(this, translations::trError, trInvalidPattern);
    count_spin_edit_->setFocus();
    return;
  }

  base_class::accept();
}

void LoadContentDbDialog::retranslateUi() {
  keys_count_label_->setText(trKeysCount + ":");
  key_pattern_label_->setText(trPattern + ":");
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
