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

#include <common/value.h>

#include "gui/models/items/action_table_item.h"

namespace fastonosql {
namespace gui {

class ValueTableItem : public ActionTableItem {
 public:
  typedef common::Value::string_t value_t;
  typedef ActionTableItem base_class;

  ValueTableItem(const value_t& value, Mode state);

  value_t value() const;
  void setValue(const value_t& val);

 private:
  value_t value_;
};

}  // namespace gui
}  // namespace fastonosql
