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

#pragma once

#include <common/value.h>

#include "gui/models/items/action_table_item.h"

namespace fastonosql {
namespace gui {

class KeyValueTableItem : public ActionTableItem {
 public:
  typedef common::Value::string_t key_t;
  typedef common::Value::string_t value_t;
  typedef ActionTableItem base_class;

  KeyValueTableItem(const key_t& key, const key_t& value, Mode state);

  key_t key() const;
  void setKey(const key_t& key);

  value_t value() const;
  void setValue(const value_t& val);

 private:
  key_t key_;
  value_t value_;
};

}  // namespace gui
}  // namespace fastonosql
