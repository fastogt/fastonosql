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

#pragma once

#include <QString>

#include "gui/action_table_item.h"

namespace fastonosql {
namespace gui {

class ValueTableItem : public ActionTableItem {
 public:
  typedef ActionTableItem base_class;
  enum eColumn : uint8_t { kValue = 0, kAction = 1, kCountColumns = 2 };

  ValueTableItem(const QString& value, Mode state);

  QString value() const;
  void setValue(const QString& val);

 private:
  QString value_;
};

}  // namespace gui
}  // namespace fastonosql
