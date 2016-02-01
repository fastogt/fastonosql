/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/load_contentdb_dialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QSpinBox>

#include "gui/gui_factory.h"
#include "translations/global.h"

namespace fastonosql {

LoadContentDbDialog::LoadContentDbDialog(const QString &title,
                                         connectionTypes type, QWidget* parent)
  : QDialog(parent), type_(type) {
  setWindowIcon(GuiFactory::instance().icon(type_));
  setWindowTitle(title);
  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &LoadContentDbDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &LoadContentDbDialog::reject));

  QVBoxLayout *mainLayout = new QVBoxLayout;

  QHBoxLayout* countLayout = new QHBoxLayout;
  countLayout->addWidget(new QLabel(tr("Keys count:")));
  countSpinEdit_ = new QSpinBox;
  countSpinEdit_->setRange(min_key_on_page, max_key_on_page);
  countSpinEdit_->setSingleStep(step_keys_on_page);
  countSpinEdit_->setValue(defaults_key);
  countLayout->addWidget(countSpinEdit_);
  mainLayout->addLayout(countLayout);

  QHBoxLayout* patternLayout = new QHBoxLayout;
  patternLayout->addWidget(new QLabel(tr("Pattern:")));
  patternEdit_ = new QLineEdit;
  patternEdit_->setFixedWidth(80);
  patternEdit_->setText("*");
  patternLayout->addWidget(patternEdit_);
  mainLayout->addLayout(patternLayout);

  mainLayout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainLayout);
}

uint32_t LoadContentDbDialog::count() const {
  return countSpinEdit_->value();
}

QString LoadContentDbDialog::pattern() const {
  return patternEdit_->text();
}

void LoadContentDbDialog::accept() {
  QString pattern = patternEdit_->text();
  if (pattern.isEmpty()) {
    QMessageBox::warning(this, translations::trError, QObject::tr("Invalid pattern!"));
    countSpinEdit_->setFocus();
    return;
  }

  QDialog::accept();
}

}  // namespace fastonosql
