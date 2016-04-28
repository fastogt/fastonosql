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

#include <vector>
#include <string>

#include "common/string_util.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace core {

std::string KeyInfo::key() const {
  return JoinString(splited_namespaces_and_key, ns_separator);
}

bool KeyInfo::hasNamespace() const {
  size_t ns_size = nspaceSize();
  return ns_size > 0;
}

std::string KeyInfo::nspace() const {
  return joinNamespace(splited_namespaces_and_key.size() - 1);
}

size_t KeyInfo::nspaceSize() const {
  if (splited_namespaces_and_key.empty()) {
    return 0;
  }

  return splited_namespaces_and_key.size() - 1;
}

std::string KeyInfo::joinNamespace(size_t pos) const {
  size_t ns_size = nspaceSize();
  if (ns_size > pos) {
    std::vector<std::string> copy;
    for (size_t i = 0; i <= pos; ++i) {
      copy.push_back(splited_namespaces_and_key[i]);
    }
    return JoinString(copy, ns_separator);
  }

  return std::string();
}

NKey::NKey(const std::string& key, ttl_t ttl_sec)
  : key(key), ttl_sec(ttl_sec) {
}

KeyInfo NKey::info(const std::string& ns_separator) const {
  std::vector<std::string> tokens;
  Tokenize(key, ns_separator, &tokens);
  return KeyInfo{tokens, ns_separator};
}

NDbKValue::NDbKValue(const NKey& key, NValue value)
  : key_(key), value_(value) {
}

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
  key_.ttl_sec = ttl;
}

void NDbKValue::setValue(NValue value) {
  value_ = value;
}

std::string NDbKValue::keyString() const {
  return key_.key;
}

}  // namespace core
}  // namespace fastonosql
