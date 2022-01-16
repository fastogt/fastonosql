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

#include "gui/models/items/fasto_common_item.h"

#include <common/qt/convert2string.h>

#include <fastonosql/core/value.h>

namespace fastonosql {
namespace gui {

FastoCommonItem::FastoCommonItem(const core::NDbKValue& key,
                                 const std::string& delimiter,
                                 bool read_only,
                                 TreeItem* parent,
                                 void* internal_pointer)
    : TreeItem(parent, internal_pointer), key_(key), delimiter_(delimiter), read_only_(read_only) {}

QString FastoCommonItem::key() const {
  QString qkey;
  const core::NKey key = key_.GetKey();
  const auto raw_key = key.GetKey();
  common::ConvertFromBytes(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

QString FastoCommonItem::readableValue() const {
  const core::NValue nval = key_.GetValue();
  const core::readable_string_t readable = core::ConvertValueForCommandLine(nval.get(), delimiter_);

  QString qval;
  common::ConvertFromBytes(readable, &qval);
  return qval;
}

void FastoCommonItem::setValue(core::NValue val) {
  key_.SetValue(val);
}

core::NValue FastoCommonItem::nvalue() const {
  return key_.GetValue();
}

core::NDbKValue FastoCommonItem::dbv() const {
  return key_;
}

common::Value::Type FastoCommonItem::type() const {
  return key_.GetType();
}

bool FastoCommonItem::isReadOnly() const {
  return read_only_;
}

core::readable_string_t toRaw(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return core::readable_string_t();
  }

  if (!item->childrenCount()) {
    const auto val = item->nvalue();
    return val.GetData();
  }

  core::readable_string_t value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toRaw(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

}  // namespace gui
}  // namespace fastonosql
