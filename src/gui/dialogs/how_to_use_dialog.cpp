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

#include "gui/dialogs/how_to_use_dialog.h"

#include <QVBoxLayout>

#include "translations/global.h"

#include "gui/widgets/how_to_use_widget.h"

namespace fastonosql {
namespace gui {

HowToUseDialog::HowToUseDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(translations::trHowToUse + " " PROJECT_NAME_TITLE);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  const QSize fixed_size(fix_width, fix_height);
  QVBoxLayout* mainLayout = new QVBoxLayout;
  HowToUseWidget* hw = new HowToUseWidget(fixed_size);
  mainLayout->addWidget(hw);
  setFixedSize(fixed_size);
  setLayout(mainLayout);
}

}  // namespace gui
}  // namespace fastonosql
