/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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
#include <QTextBrowser>

#include <common/macros.h>

namespace {
const QString trEulaTitle = QObject::tr("Eula for " PROJECT_NAME_TITLE);
const QString trIAgree = QObject::tr("I agree");
const QString trIDontAgree = QObject::tr("I don't agree");
}  // namespace

namespace fastonosql {
namespace gui {

EulaDialog::EulaDialog(QWidget* parent) : QWizard(parent) {
  setWindowTitle(trEulaTitle);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  //// First page
  QWizardPage* firstPage = new QWizardPage;

  QRadioButton* agreeButton = new QRadioButton(trIAgree);
  VERIFY(connect(agreeButton, SIGNAL(clicked()), this, SLOT(on_agreeButton_clicked())));

  QRadioButton* notAgreeButton = new QRadioButton(trIDontAgree);
  notAgreeButton->setChecked(true);
  VERIFY(connect(notAgreeButton, SIGNAL(clicked()), this, SLOT(on_notAgreeButton_clicked())));

  QHBoxLayout* radioButtonsLay = new QHBoxLayout;
  radioButtonsLay->setAlignment(Qt::AlignHCenter);
  radioButtonsLay->setSpacing(30);
  radioButtonsLay->addWidget(agreeButton);
  radioButtonsLay->addWidget(notAgreeButton);

  QTextBrowser* textBrowser = new QTextBrowser;
  textBrowser->setOpenExternalLinks(true);
  textBrowser->setOpenLinks(true);
  QFile file(":" PROJECT_NAME_LOWERCASE "/LICENSE");
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    textBrowser->setText(file.readAll());
  }

  QFrame* hline = new QFrame;
  hline->setFrameShape(QFrame::HLine);
  hline->setFrameShadow(QFrame::Sunken);

  QVBoxLayout* mainLayout1 = new QVBoxLayout;
  mainLayout1->addWidget(new QLabel("<h3>End-User License Agreement</h3>"));
  mainLayout1->addWidget(new QLabel(""));
  mainLayout1->addWidget(textBrowser);
  mainLayout1->addWidget(new QLabel(""));
  mainLayout1->addLayout(radioButtonsLay, Qt::AlignCenter);
  mainLayout1->addWidget(new QLabel(""));
  mainLayout1->addWidget(hline);

  firstPage->setLayout(mainLayout1);

  addPage(firstPage);

  //// Buttons
  setButtonText(QWizard::CustomButton1, tr("Back"));
  setButtonText(QWizard::CustomButton2, tr("Next"));
  setButtonText(QWizard::CustomButton3, tr("Finish"));

  VERIFY(connect(button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(on_back_clicked())));
  VERIFY(connect(button(QWizard::CustomButton2), SIGNAL(clicked()), this, SLOT(on_next_clicked())));
  VERIFY(connect(button(QWizard::CustomButton3), SIGNAL(clicked()), this, SLOT(on_finish_clicked())));

  setButtonLayout(QList<WizardButton>{QWizard::Stretch, QWizard::CustomButton1, QWizard::CustomButton2,
                                      QWizard::CancelButton, QWizard::CustomButton3});

#if defined(_WIN32) || defined(__APPLE__)
  button(QWizard::CustomButton1)->setDisabled(true);
  button(QWizard::CustomButton2)->setDisabled(true);
  button(QWizard::CustomButton3)->setDisabled(true);
#else  // linux
  button(QWizard::CustomButton1)->setHidden(true);
  button(QWizard::CustomButton2)->setHidden(true);
  button(QWizard::CustomButton3)->setDisabled(true);
#endif

  setWizardStyle(QWizard::ModernStyle);
}

void EulaDialog::on_agreeButton_clicked() {
#if defined(_WIN32) || defined(__APPLE__)
  button(QWizard::CustomButton2)->setEnabled(true);
#else
  button(QWizard::CustomButton3)->setEnabled(true);
#endif
}

void EulaDialog::on_notAgreeButton_clicked() {
#if defined(_WIN32) || defined(__APPLE__)
  button(QWizard::CustomButton2)->setEnabled(false);
#else
  button(QWizard::CustomButton3)->setEnabled(false);
#endif
}

void EulaDialog::on_next_clicked() {
  next();
  button(QWizard::CustomButton1)->setEnabled(true);
  button(QWizard::CustomButton2)->setEnabled(false);
  button(QWizard::CustomButton3)->setEnabled(true);
}

void EulaDialog::on_back_clicked() {
  back();
  button(QWizard::CustomButton1)->setEnabled(false);
  button(QWizard::CustomButton2)->setEnabled(true);
  button(QWizard::CustomButton3)->setEnabled(false);
}

void EulaDialog::on_finish_clicked() {
  accept();
}

}  // namespace gui
}  // namespace fastonosql
