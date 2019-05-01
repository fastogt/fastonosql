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

#include "gui/dialogs/eula_dialog.h"

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTextEdit>

#include <common/macros.h>

namespace {
const QString trIAgree = QObject::tr("I agree");
const QString trIDontAgree = QObject::tr("I don't agree");
const QString trBack = QObject::tr("Back");
const QString trNext = QObject::tr("Next");
const QString trFinish = QObject::tr("Finish");
const QString trEndUserAgr = QObject::tr("End-User License Agreement");
}  // namespace

namespace fastonosql {
namespace gui {

EulaDialog::EulaDialog(const QString& title, QWidget* parent) : QWizard(parent) {
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  //// First page
  QWizardPage* first_page = new QWizardPage;

  QRadioButton* agree_button = new QRadioButton(trIAgree);
  VERIFY(connect(agree_button, &QRadioButton::clicked, this, &EulaDialog::agreeClick));

  QRadioButton* not_agree_button = new QRadioButton(trIDontAgree);
  not_agree_button->setChecked(true);
  VERIFY(connect(not_agree_button, &QRadioButton::clicked, this, &EulaDialog::notAgreeClick));

  QHBoxLayout* radio_buttons_layout = new QHBoxLayout;
  radio_buttons_layout->setAlignment(Qt::AlignHCenter);
  radio_buttons_layout->setSpacing(30);
  radio_buttons_layout->addWidget(agree_button);
  radio_buttons_layout->addWidget(not_agree_button);

  QTextEdit* text_browser = new QTextEdit;
  QFile file(":" PROJECT_NAME_LOWERCASE "/LICENSE");
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    text_browser->setHtml(file.readAll());
  }

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(new QLabel("<h3>" + trEndUserAgr + "</h3>"));
  main_layout->addWidget(text_browser);
  main_layout->addLayout(radio_buttons_layout, Qt::AlignCenter);
  first_page->setLayout(main_layout);

  addPage(first_page);

  //// Buttons
  setButtonText(QWizard::CustomButton1, trBack);
  setButtonText(QWizard::CustomButton2, trNext);
  setButtonText(QWizard::CustomButton3, trFinish);

  VERIFY(connect(button(QWizard::CustomButton1), &QAbstractButton::clicked, this, &EulaDialog::backClick));
  VERIFY(connect(button(QWizard::CustomButton2), &QAbstractButton::clicked, this, &EulaDialog::nextClick));
  VERIFY(connect(button(QWizard::CustomButton3), &QAbstractButton::clicked, this, &EulaDialog::finishClick));

  setButtonLayout(QList<WizardButton>{QWizard::Stretch, QWizard::CustomButton1, QWizard::CustomButton2,
                                      QWizard::CancelButton, QWizard::CustomButton3});

  button(QWizard::CustomButton1)->setHidden(true);
  button(QWizard::CustomButton2)->setHidden(true);
  button(QWizard::CustomButton3)->setDisabled(true);
  setWizardStyle(QWizard::ModernStyle);
}

void EulaDialog::agreeClick() {
  button(QWizard::CustomButton3)->setEnabled(true);
}

void EulaDialog::notAgreeClick() {
  button(QWizard::CustomButton3)->setEnabled(false);
}

void EulaDialog::nextClick() {
  next();
  button(QWizard::CustomButton1)->setEnabled(true);
  button(QWizard::CustomButton2)->setEnabled(false);
  button(QWizard::CustomButton3)->setEnabled(true);
}

void EulaDialog::backClick() {
  back();
  button(QWizard::CustomButton1)->setEnabled(false);
  button(QWizard::CustomButton2)->setEnabled(true);
  button(QWizard::CustomButton3)->setEnabled(false);
}

void EulaDialog::finishClick() {
  accept();
}

}  // namespace gui
}  // namespace fastonosql
