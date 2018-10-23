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

#include "gui/dialogs/eula_dialog.h"

#include <QFile>
#include <QFrame>
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
  QWizardPage* firstPage = new QWizardPage;

  QRadioButton* agreeButton = new QRadioButton(trIAgree);
  VERIFY(connect(agreeButton, &QRadioButton::clicked, this, &EulaDialog::agreeButtonClicked));

  QRadioButton* notAgreeButton = new QRadioButton(trIDontAgree);
  notAgreeButton->setChecked(true);
  VERIFY(connect(notAgreeButton, &QRadioButton::clicked, this, &EulaDialog::notAgreeButtonClicked));

  QHBoxLayout* radioButtonsLay = new QHBoxLayout;
  radioButtonsLay->setAlignment(Qt::AlignHCenter);
  radioButtonsLay->setSpacing(30);
  radioButtonsLay->addWidget(agreeButton);
  radioButtonsLay->addWidget(notAgreeButton);

  QTextEdit* textBrowser = new QTextEdit;
  QFile file(":" PROJECT_NAME_LOWERCASE "/LICENSE");
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    textBrowser->setHtml(file.readAll());
  }

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(new QLabel("<h3>" + trEndUserAgr + "</h3>"));
  main_layout->addWidget(textBrowser);
  main_layout->addLayout(radioButtonsLay, Qt::AlignCenter);
  firstPage->setLayout(main_layout);

  addPage(firstPage);

  //// Buttons
  setButtonText(QWizard::CustomButton1, trBack);
  setButtonText(QWizard::CustomButton2, trNext);
  setButtonText(QWizard::CustomButton3, trFinish);

  VERIFY(connect(button(QWizard::CustomButton1), &QAbstractButton::clicked, this, &EulaDialog::backButtonClicked));
  VERIFY(connect(button(QWizard::CustomButton2), &QAbstractButton::clicked, this, &EulaDialog::nextButtonClicked));
  VERIFY(connect(button(QWizard::CustomButton3), &QAbstractButton::clicked, this, &EulaDialog::finishButtonClicked));

  setButtonLayout(QList<WizardButton>{QWizard::Stretch, QWizard::CustomButton1, QWizard::CustomButton2,
                                      QWizard::CancelButton, QWizard::CustomButton3});

  button(QWizard::CustomButton1)->setHidden(true);
  button(QWizard::CustomButton2)->setHidden(true);
  button(QWizard::CustomButton3)->setDisabled(true);
  setWizardStyle(QWizard::ModernStyle);
}

void EulaDialog::agreeButtonClicked() {
  button(QWizard::CustomButton3)->setEnabled(true);
}

void EulaDialog::notAgreeButtonClicked() {
  button(QWizard::CustomButton3)->setEnabled(false);
}

void EulaDialog::nextButtonClicked() {
  next();
  button(QWizard::CustomButton1)->setEnabled(true);
  button(QWizard::CustomButton2)->setEnabled(false);
  button(QWizard::CustomButton3)->setEnabled(true);
}

void EulaDialog::backButtonClicked() {
  back();
  button(QWizard::CustomButton1)->setEnabled(false);
  button(QWizard::CustomButton2)->setEnabled(true);
  button(QWizard::CustomButton3)->setEnabled(false);
}

void EulaDialog::finishButtonClicked() {
  accept();
}

}  // namespace gui
}  // namespace fastonosql
