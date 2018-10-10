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

#include "gui/dialogs/connection_select_type_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"

namespace {
const QString trSectTypeTitle = QObject::tr("Select connection type");
const QString trDatabase = QObject::tr("Database");
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionSelectTypeDialog::ConnectionSelectTypeDialog(QWidget* parent) : QDialog(parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  type_connection_label_ = new QLabel;
  type_connection_ = new QComboBox;

  for (size_t i = 0; i < core::g_compiled_types.size(); ++i) {
    core::ConnectionType ct = core::g_compiled_types[i];
    std::string str = common::ConvertToString(ct);
    QString qstr;
    if (common::ConvertFromString(str, &qstr)) {
      type_connection_->addItem(GuiFactory::GetInstance().icon(ct), qstr, ct);
    }
  }

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QHBoxLayout* typeLayout = new QHBoxLayout;
  typeLayout->addWidget(type_connection_label_);
  typeLayout->addWidget(type_connection_);
  mainLayout->addLayout(typeLayout);

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &ConnectionSelectTypeDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &ConnectionSelectTypeDialog::reject));
  bottomLayout->addWidget(button_box_);
  mainLayout->addLayout(bottomLayout);

  mainLayout->setSizeConstraint(QLayout::SetFixedSize);

  setLayout(mainLayout);

  retranslateUi();
}

core::ConnectionType ConnectionSelectTypeDialog::connectionType() const {
  QVariant var = type_connection_->currentData();
  return static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));
}

void ConnectionSelectTypeDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ConnectionSelectTypeDialog::retranslateUi() {
  setWindowTitle(trSectTypeTitle);
  type_connection_label_->setText(trDatabase + ":");
}

}  // namespace gui
}  // namespace fastonosql
