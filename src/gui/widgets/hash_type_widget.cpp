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

#include "gui/widgets/hash_type_widget.h"

#include <QToolButton>

#include <common/macros.h>
#include <common/qt/convert2string.h>

#include <gui/gui_factory.h>

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HashTypeWidget::HashTypeWidget(QWidget* parent) : QTableWidget(0, 3, parent) {}

void HashTypeWidget::insertRow(QTableWidgetItem* key, QTableWidgetItem* value) {
  QTableWidget::insertRow(0);
  setItem(0, 0, key);
  setItem(0, 1, value);

  QToolButton* butt = new QToolButton;
  butt->setIcon(GuiFactory::instance().removeIcon());
  setCellWidget(0, 2, butt);
  setColumnWidth(2, rowHeight(0));

  VERIFY(connect(butt, &QToolButton::clicked, this, &HashTypeWidget::removeRowInner));
}

common::ZSetValue* HashTypeWidget::zsetValue() const {
  common::ZSetValue* ar = common::Value::createZSetValue();
  for (int i = 0; i < rowCount(); ++i) {
    QTableWidgetItem* kitem = item(i, 0);
    QTableWidgetItem* vitem = item(i, 1);

    std::string key = common::ConvertToString(kitem->text());
    std::string val = common::ConvertToString(vitem->text());
    ar->insert(key, val);
  }

  return ar;
}

common::HashValue* HashTypeWidget::hashValue() const {
  common::HashValue* ar = common::Value::createHashValue();
  for (int i = 0; i < rowCount(); ++i) {
    QTableWidgetItem* kitem = item(i, 0);
    QTableWidgetItem* vitem = item(i, 1);

    std::string key = common::ConvertToString(kitem->text());
    std::string val = common::ConvertToString(vitem->text());
    ar->insert(key, val);
  }

  return ar;
}

void HashTypeWidget::removeRowInner() {
  QToolButton* butt = qobject_cast<QToolButton*>(sender());
  for (int i = 0; i < rowCount(); ++i) {
    QWidget* wid = cellWidget(i, 2);
    if (wid == butt) {
      removeRow(i);
      return;
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
