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

#include "gui/dialogs/how_to_use_dialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <common/macros.h>

#include "gui/widgets/how_to_use_widget.h"

#include "translations/global.h"

namespace {
const QSize kFixedSize = QSize(1280 / 2, 720 / 2);  // 2x smaller than original
}

namespace fastonosql {
namespace gui {

HowToUseDialog::HowToUseDialog(QWidget* parent)
    : base_class(translations::trHowToUse + " " PROJECT_NAME_TITLE, parent) {
  HowToUseWidget* hw = new HowToUseWidget(kFixedSize);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &HowToUseDialog::accept));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(hw);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setFixedSize(kFixedSize);
}

}  // namespace gui
}  // namespace fastonosql
