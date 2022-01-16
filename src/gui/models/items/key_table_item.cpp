/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "gui/models/items/key_table_item.h"

#include <common/qt/convert2string.h>

#include <fastonosql/core/value.h>

namespace fastonosql {
namespace gui {

KeyTableItem::KeyTableItem(const core::NDbKValue& dbv) : dbv_(dbv) {}

QString KeyTableItem::keyString() const {
  QString qkey;
  const core::NKey key = dbv_.GetKey();
  const auto raw_key = key.GetKey();
  common::ConvertFromBytes(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

QString KeyTableItem::typeText() const {
  return core::GetTypeName(dbv_.GetType());
}

core::ttl_t KeyTableItem::TTL() const {
  core::NKey key = dbv_.GetKey();
  return key.GetTTL();
}

common::Value::Type KeyTableItem::type() const {
  return dbv_.GetType();
}

core::NDbKValue KeyTableItem::dbv() const {
  return dbv_;
}

void KeyTableItem::setDbv(const core::NDbKValue& val) {
  dbv_ = val;
}

core::NKey KeyTableItem::key() const {
  return dbv_.GetKey();
}

void KeyTableItem::setKey(const core::NKey& key) {
  dbv_.SetKey(key);
}

}  // namespace gui
}  // namespace fastonosql
