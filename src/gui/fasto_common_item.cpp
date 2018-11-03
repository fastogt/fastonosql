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

#include "gui/fasto_common_item.h"

#include <common/qt/convert2string.h>

namespace fastonosql {
namespace gui {

FastoCommonItem::FastoCommonItem(const core::NDbKValue& key,
                                 const std::string& delimiter,
                                 bool read_only,
                                 TreeItem* parent,
                                 void* internalPointer)
    : TreeItem(parent, internalPointer), key_(key), delimiter_(delimiter), read_only_(read_only) {}

QString FastoCommonItem::key() const {
  QString qkey;
  const core::NKey key = key_.GetKey();
  const core::key_t raw_key = key.GetKey();
  common::ConvertFromBytes(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

core::ReadableString FastoCommonItem::coreValue() const {
  core::NValue nval = key_.GetValue();
  return nval.GetReadableValue(delimiter_);
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

const char* FastoCommonItem::delimiter() const {
  return delimiter_.c_str();
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
    const auto val = item->coreValue();
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
