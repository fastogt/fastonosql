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

#include "gui/widgets/list_type_widget.h"

#include <common/macros.h>
#include <common/qt/convert2string.h>

#include "gui/widgets/item_cell_delegate.h"

#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {

ListTypeWidget::ListTypeWidget(QWidget* parent) : QListWidget(parent) {
  ItemCellDelegate* del = new ItemCellDelegate(this);
  del->setButtonIcon(GuiFactory::instance().removeIcon());
  setItemDelegate(del);

  VERIFY(connect(del, &ItemCellDelegate::buttonClicked, this, &ListTypeWidget::removeItem));
}

common::ArrayValue* ListTypeWidget::arrayValue() const {
  common::ArrayValue* ar = common::Value::createArrayValue();
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem* it = item(i);
    std::string val = common::ConvertToString(it->text());
    ar->appendString(val);
  }

  return ar;
}

common::SetValue* ListTypeWidget::setValue() const {
  common::SetValue* set = common::Value::createSetValue();
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem* it = item(i);
    std::string val = common::ConvertToString(it->text());
    set->insert(val);
  }

  return set;
}

void ListTypeWidget::addEmptyItem() {
  insertItem(0, QString());
}

void ListTypeWidget::removeCurrentItem() {
  QListWidgetItem* cur = currentItem();
  removeItemWidget(cur);
}

void ListTypeWidget::removeItem(int row) {
  QListWidgetItem* item = takeItem(row);
  delete item;
}

}  // namespace gui
}  // namespace fastonosql
