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

#include "gui/key_value_table_item.h"

namespace fastonosql {
namespace gui {

KeyValueTableItem::KeyValueTableItem(const QString& key, const QString& value, Mode state)
    : TableItem(), key_(key), value_(value), state_(state) {}

QString KeyValueTableItem::GetKey() const {
  return key_;
}

void KeyValueTableItem::SetKey(const QString& key) {
  key_ = key;
}

QString KeyValueTableItem::GetValue() const {
  return value_;
}

void KeyValueTableItem::SetValue(const QString& val) {
  value_ = val;
}

KeyValueTableItem::Mode KeyValueTableItem::GetActionState() const {
  return state_;
}

}  // namespace gui
}  // namespace fastonosql
