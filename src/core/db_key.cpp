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

bool IsBinaryKey(const std::string& key) {
  for (size_t i = 0; i < key.size(); ++i) {
    char c = key[i];
    if (isprint(c) == 0) {  // should be hexed symbol
      return true;
    }
  }
  return false;
}

KeyString::KeyString() : key_(), type_(TEXT_KEY) {}

KeyString::KeyString(const common::string_byte_writer& key_data) : key_(), type_() {
  SetKey(key_data);
}

KeyString::KeyType KeyString::GetType() const {
  return type_;
}

string_key_t KeyString::GetKey() const {
  return key_;
}

void KeyString::SetKey(const common::string_byte_writer& key_data) {
  SetKey(key_data.GetBuffer());
}

const string_key_t::value_type* KeyString::GetKeyData() const {
  return key_.c_str();
}

string_key_t::size_type KeyString::GetKeySize() const {
  return key_.size();
}

void KeyString::SetKey(string_key_t new_key) {
  key_ = new_key;
  type_ = IsBinaryKey(new_key) ? BINARY_KEY : TEXT_KEY;
}

std::string KeyString::ToString() const {
  if (type_ == BINARY_KEY) {
    return common::utils::hex::encode(key_, true);
  }

  return key_;
}

bool KeyString::Equals(const KeyString& other) const {
  if (key_ != other.key_) {
    return false;
  }

  return type_ == other.type_;
}

KeyString KeyString::MakeKeyString(string_key_t str) {
  KeyString ks;
  ks.SetKey(str);
  return ks;
}

KeyInfo::KeyInfo(const splited_namespaces_t& splited_namespaces_and_key, string_key_t ns_separator)
    : splited_namespaces_and_key_(splited_namespaces_and_key), ns_separator_(ns_separator) {}

string_key_t KeyInfo::GetKey() const {
  return common::JoinString(splited_namespaces_and_key_, ns_separator_);
}

bool KeyInfo::HasNamespace() const {
  size_t ns_size = GetNspaceSize();
  return ns_size > 0;
}

string_key_t KeyInfo::GetNspace() const {
  return JoinNamespace(splited_namespaces_and_key_.size() - 1);
}

size_t KeyInfo::GetNspaceSize() const {
  if (splited_namespaces_and_key_.empty()) {
    return 0;
  }

  return splited_namespaces_and_key_.size() - 1;
}

string_key_t KeyInfo::JoinNamespace(size_t pos) const {
  size_t ns_size = GetNspaceSize();
  if (ns_size > pos) {
    std::vector<string_key_t> copy;
    for (size_t i = 0; i <= pos; ++i) {
      copy.push_back(splited_namespaces_and_key_[i]);
    }
    return common::JoinString(copy, ns_separator_);
  }

  return string_key_t();
}

NKey::NKey() : key_(), ttl_(NO_TTL) {}

NKey::NKey(key_t key, ttl_t ttl_sec) : key_(key), ttl_(ttl_sec) {}

KeyInfo NKey::GetInfo(string_key_t ns_separator) const {
  KeyInfo::splited_namespaces_t tokens;
  common::Tokenize(key_.ToString(), ns_separator, &tokens);
  return KeyInfo(tokens, ns_separator);
}

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
