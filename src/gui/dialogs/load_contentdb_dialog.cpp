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

#include "gui/dialogs/load_contentdb_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trError

namespace {
const QString trInvalidPattern = QObject::tr("Invalid pattern!");
const QString trKeysCount = QObject::tr("Keys count");
const QString trPattern = QObject::tr("Pattern");
const QString defaultPattern = ALL_KEYS_PATTERNS;
}  // namespace

namespace fastonosql {
namespace gui {

LoadContentDbDialog::LoadContentDbDialog(const QString& title, core::connectionTypes type, QWidget* parent)
    : QDialog(parent), type_(type) {
  setWindowTitle(title);
  setWindowIcon(GuiFactory::GetInstance().GetIcon(type_));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &LoadContentDbDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &LoadContentDbDialog::reject));

  QVBoxLayout* mainLayout = new QVBoxLayout;

  QHBoxLayout* countLayout = new QHBoxLayout;
  countLayout->addWidget(new QLabel(trKeysCount + ":"));
  count_spin_edit_ = new QSpinBox;
  count_spin_edit_->setRange(min_key_on_page, max_key_on_page);
  count_spin_edit_->setSingleStep(step_keys_on_page);
  count_spin_edit_->setValue(defaults_key);
  countLayout->addWidget(count_spin_edit_);
  mainLayout->addLayout(countLayout);

  QHBoxLayout* patternLayout = new QHBoxLayout;
  patternLayout->addWidget(new QLabel(trPattern + ":"));
  pattern_edit_ = new QLineEdit;
  pattern_edit_->setFixedWidth(80);
  pattern_edit_->setText(defaultPattern);
  patternLayout->addWidget(pattern_edit_);
  mainLayout->addLayout(patternLayout);

  mainLayout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainLayout);
}

int LoadContentDbDialog::count() const {
  return count_spin_edit_->value();
}

QString LoadContentDbDialog::pattern() const {
  return pattern_edit_->text();
}

void LoadContentDbDialog::accept() {
  QString pattern = pattern_edit_->text();
  if (pattern.isEmpty()) {
    QMessageBox::warning(this, translations::trError, trInvalidPattern);
    count_spin_edit_->setFocus();
    return;
  }

  QDialog::accept();
}

}  // namespace gui
}  // namespace fastonosql
