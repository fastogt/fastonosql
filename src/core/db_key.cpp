/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/string_util.h>  // for JoinString, Tokenize

#include "global/global.h"

namespace fastonosql {
namespace core {

KeyInfo::KeyInfo(const splited_namespaces_t& splited_namespaces_and_key,
                 const std::string& ns_separator)
    : splited_namespaces_and_key_(splited_namespaces_and_key), ns_separator_(ns_separator) {}

std::string KeyInfo::key() const {
  return common::JoinString(splited_namespaces_and_key_, ns_separator_);
}

bool KeyInfo::hasNamespace() const {
  size_t ns_size = nspaceSize();
  return ns_size > 0;
}

std::string KeyInfo::nspace() const {
  return joinNamespace(splited_namespaces_and_key_.size() - 1);
}

size_t KeyInfo::nspaceSize() const {
  if (splited_namespaces_and_key_.empty()) {
    return 0;
  }

  return splited_namespaces_and_key_.size() - 1;
}

std::string KeyInfo::joinNamespace(size_t pos) const {
  size_t ns_size = nspaceSize();
  if (ns_size > pos) {
    std::vector<std::string> copy;
    for (size_t i = 0; i <= pos; ++i) {
      copy.push_back(splited_namespaces_and_key_[i]);
    }
    return common::JoinString(copy, ns_separator_);
  }

  return std::string();
}

NKey::NKey() : key_(), ttl_(NO_TTL) {}

NKey::NKey(const std::string& key, ttl_t ttl_sec) : key_(key), ttl_(ttl_sec) {}

KeyInfo NKey::info(const std::string& ns_separator) const {
  KeyInfo::splited_namespaces_t tokens;
  common::Tokenize(key_, ns_separator, &tokens);
  return KeyInfo(tokens, ns_separator);
}

std::string NKey::key() const {
  return key_;
}

void NKey::setKey(const std::string& key) {
  key_ = key;
}

ttl_t NKey::ttl() const {
  return ttl_;
}

void NKey::setTTL(ttl_t ttl) {
  ttl_ = ttl;
}

bool NKey::equals(const NKey& other) const {
  if (key_ != other.key_) {
    return false;
  }

  return ttl_ == other.ttl_;
}

NDbKValue::NDbKValue() : key_(), value_() {}

NDbKValue::NDbKValue(const NKey& key, NValue value) : key_(key), value_(value) {}

NKey NDbKValue::key() const {
  return key_;
}

NValue NDbKValue::value() const {
  return value_;
}

common::Value::Type NDbKValue::type() const {
  if (!value_) {
    return common::Value::TYPE_NULL;
  }

  return value_->type();
}

void NDbKValue::setTTL(ttl_t ttl) {
  key_.setTTL(ttl);
}

void NDbKValue::setKey(const NKey& key) {
  key_ = key;
}

void NDbKValue::setValue(NValue value) {
  value_ = value;
}

std::string NDbKValue::keyString() const {
  return key_.key();
}

std::string NDbKValue::valueString() const {
  return common::ConvertToString(value_.get(), " ");
}

bool NDbKValue::equals(const NDbKValue& other) const {
  if (!key_.equals(other.key_)) {
    return false;
  }

  return value_->equals(other.value_.get());
}

}  // namespace core
}  // namespace fastonosql
