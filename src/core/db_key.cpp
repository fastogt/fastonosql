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

#include "core/db_key.h"

#include <memory>  // for __shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <common/convert2string.h>
#include <common/string_util.h>  // for JoinString, Tokenize

#include "core/global.h"

namespace fastonosql {
namespace core {

bool IsBinaryKey(const command_buffer_t& key) {
  for (size_t i = 0; i < key.size(); ++i) {
    char c = key[i];
    if (isprint(c) == 0) {  // should be hexed symbol
      return true;
    }
  }
  return false;
}

KeyString::KeyString() : key_(), type_(TEXT_KEY) {}

KeyString::KeyString(const string_key_t& key_data) : key_(), type_(TEXT_KEY) {
  SetKeyData(key_data);
}

KeyString::KeyType KeyString::GetType() const {
  return type_;
}

std::string KeyString::ToBytes() const {
  return key_;
}

std::string KeyString::ToString() const {
  if (type_ == BINARY_KEY) {
    command_buffer_writer_t wr;
    string_key_t hexed = common::utils::hex::encode(key_, false);
    for (size_t i = 0; i < hexed.size(); i += 2) {
      wr << MAKE_COMMAND_BUFFER("\\x");
      wr << hexed[i];
      wr << hexed[i + 1];
    }

    return wr.str();
  }

  return key_;
}

string_key_t KeyString::GetKeyData() const {
  if (type_ == BINARY_KEY) {
    command_buffer_writer_t wr;
    wr << "\"";
    string_key_t hexed = common::utils::hex::encode(key_, false);
    for (size_t i = 0; i < hexed.size(); i += 2) {
      wr << MAKE_COMMAND_BUFFER("\\x");
      wr << hexed[i];
      wr << hexed[i + 1];
    }
    wr << "\"";

    return wr.str();
  }

  return key_;
}

void KeyString::SetKeyData(const string_key_t& key_data) {
  key_ = key_data;
  type_ = IsBinaryKey(key_data) ? BINARY_KEY : TEXT_KEY;
}

bool KeyString::Equals(const KeyString& other) const {
  return key_ == other.key_;
}

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

bool NKey::Equals(const NKey& other) const {
  if (key_ != other.key_) {
    return false;
  }

  return ttl_ == other.ttl_;
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

std::string NDbKValue::ValueString() const {
  return common::ConvertToString(value_.get(), " ");
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
