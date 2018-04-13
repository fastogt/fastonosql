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

#include "core/db_key.h"

#include <common/convert2string.h>
#include <common/string_util.h>  // for JoinString, Tokenize

#include "core/value.h"

namespace fastonosql {
namespace core {

NKey::NKey() : key_(), ttl_(NO_TTL) {}

NKey::NKey(key_t key, ttl_t ttl_sec) : key_(key), ttl_(ttl_sec) {}

key_t NKey::GetKey() const {
  return key_;
}

void NKey::SetKey(key_t key) {
  key_ = key;
}

ttl_t NKey::GetTTL() const {
  return ttl_;
}

void NKey::SetTTL(ttl_t ttl) {
  ttl_ = ttl;
}

bool NKey::EqualsKey(const NKey& key) const {
  return key_ == key.key_;
}

bool NKey::Equals(const NKey& other) const {
  if (key_ != other.key_) {
    return false;
  }

  return ttl_ == other.ttl_;
}

NValue::NValue() : base_class() {}

NValue::NValue(const base_class& other) : base_class(other) {}

value_t NValue::GetValue(const std::string& delimiter) const {
  return ConvertValue(get(), delimiter);
}

NDbKValue::NDbKValue() : key_(), value_() {}

NDbKValue::NDbKValue(const NKey& key, NValue value) : key_(key), value_(value) {}

NKey NDbKValue::GetKey() const {
  return key_;
}

NValue NDbKValue::GetValue() const {
  return value_;
}

common::Value::Type NDbKValue::GetType() const {
  if (!value_) {
    return common::Value::TYPE_NULL;
  }

  return value_->GetType();
}

void NDbKValue::SetKey(const NKey& key) {
  key_ = key;
}

void NDbKValue::SetValue(NValue value) {
  value_ = value;
}

bool NDbKValue::EqualsKey(const NKey& key) const {
  return key_.EqualsKey(key);
}

bool NDbKValue::Equals(const NDbKValue& other) const {
  if (!key_.Equals(other.key_)) {
    return false;
  }

  if (!value_) {
    return !other.value_;
  }

  return value_->Equals(other.value_.get());
}

}  // namespace core
}  // namespace fastonosql
