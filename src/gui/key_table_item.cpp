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

#include "gui/key_table_item.h"

#include <common/qt/convert2string.h>

#include "core/value.h"

namespace fastonosql {
namespace gui {

KeyTableItem::KeyTableItem(const core::NDbKValue& dbv) : dbv_(dbv) {}

QString KeyTableItem::GetKeyString() const {
  QString qkey;
  const core::NKey key = dbv_.GetKey();
  const core::key_t raw_key = key.GetKey();
  common::ConvertFromString(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

QString KeyTableItem::GetTypeText() const {
  return core::GetTypeName(dbv_.GetType());
}

core::ttl_t KeyTableItem::GetTTL() const {
  core::NKey key = dbv_.GetKey();
  return key.GetTTL();
}

common::Value::Type KeyTableItem::GetType() const {
  return dbv_.GetType();
}

core::NDbKValue KeyTableItem::GetDBV() const {
  return dbv_;
}

void KeyTableItem::SetDbv(const core::NDbKValue& val) {
  dbv_ = val;
}

core::NKey KeyTableItem::GetKey() const {
  return dbv_.GetKey();
}

void KeyTableItem::SetKey(const core::NKey& key) {
  dbv_.SetKey(key);
}

}  // namespace gui
}  // namespace fastonosql
