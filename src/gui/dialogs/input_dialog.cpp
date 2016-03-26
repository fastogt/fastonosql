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

#include "gui/dialogs/input_dialog.h"

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QGridLayout>

#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {

InputDialog::InputDialog(QWidget* parent, const QString& title, InputType type,
                       const QString& firstLabelText, const QString& secondLabelText)
  : QDialog(parent) {
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  QGridLayout* glayout = new QGridLayout;

  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &InputDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &InputDialog::reject));
  QLabel* firstLabel = new QLabel(firstLabelText);
  firstLine_ = new QLineEdit;
  secondLine_ = new QLineEdit;

  glayout->addWidget(firstLabel, 0, 0);
  glayout->addWidget(firstLine_, 0, 1);

  if (type == DoubleLine) {
    QLabel* secondLabel = new QLabel(secondLabelText);
    glayout->addWidget(secondLabel, 1, 0);
    glayout->addWidget(secondLine_, 1, 1);
  }

  glayout->addWidget(buttonBox, 2, 1);

  setLayout(glayout);
  glayout->setSizeConstraint(QLayout::SetFixedSize);
}

QString InputDialog::firstText() const {
  return firstLine_->text();
}

QString InputDialog::secondText() const {
  return secondLine_->text();
}

}  // namespace gui
}  // namespace fastonosql
