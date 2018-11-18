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

#include "gui/models/items/key_value_table_item.h"

namespace fastonosql {
namespace gui {

KeyValueTableItem::KeyValueTableItem(const QString& key, const QString& value, Mode state)
    : base_class(state), key_(key), value_(value) {}

QString KeyValueTableItem::key() const {
  return key_;
}

void KeyValueTableItem::setKey(const QString& key) {
  key_ = key;
}

QString KeyValueTableItem::value() const {
  return value_;
}

void KeyValueTableItem::setValue(const QString& val) {
  value_ = val;
}

}  // namespace gui
}  // namespace fastonosql
