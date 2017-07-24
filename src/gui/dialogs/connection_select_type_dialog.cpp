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

#include "gui/dialogs/connection_select_type_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <common/convert2string.h>
#include <common/macros.h>
#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"

namespace {
const QString trSectTypeTitle = QObject::tr("Select connection type");
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionSelectTypeDialog::ConnectionSelectTypeDialog(QWidget* parent) : QDialog(parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  typeConnectionLabel_ = new QLabel;
  typeConnection_ = new QComboBox;

  for (size_t i = 0; i < SIZEOFMASS(core::compiled_types); ++i) {
    core::connectionTypes ct = core::compiled_types[i];
    std::string str = common::ConvertToString(ct);
    QString qstr;
    if (common::ConvertFromString(str, &qstr)) {
      typeConnection_->addItem(GuiFactory::Instance().icon(ct), qstr, ct);
    }
  }

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QHBoxLayout* typeLayout = new QHBoxLayout;
  typeLayout->addWidget(typeConnectionLabel_);
  typeLayout->addWidget(typeConnection_);
  mainLayout->addLayout(typeLayout);

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox_->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &ConnectionSelectTypeDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &ConnectionSelectTypeDialog::reject));
  bottomLayout->addWidget(buttonBox_);
  mainLayout->addLayout(bottomLayout);

  mainLayout->setSizeConstraint(QLayout::SetFixedSize);

  setLayout(mainLayout);

  retranslateUi();
}

core::connectionTypes ConnectionSelectTypeDialog::connectionType() const {
  QVariant var = typeConnection_->currentData();
  return static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));
}

void ConnectionSelectTypeDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ConnectionSelectTypeDialog::retranslateUi() {
  setWindowTitle(trSectTypeTitle);
  typeConnectionLabel_->setText(tr("Database:"));
}

}  // namespace gui
}  // namespace fastonosql
