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

#include "proxy/types.h"

#include <common/string_util.h>

namespace fastonosql {
namespace proxy {

const std::vector<const char*> supported_views_text = {"Tree", "Table", "Text"};

KeyInfo::KeyInfo(const splited_namespaces_t& splited_namespaces_and_key, std::string ns_separator)
    : splited_namespaces_and_key_(splited_namespaces_and_key), ns_separator_(ns_separator) {}

std::string KeyInfo::GetKey() const {
  return common::JoinString(splited_namespaces_and_key_, ns_separator_);
}

bool KeyInfo::HasNamespace() const {
  size_t ns_size = GetNspaceSize();
  return ns_size > 0;
}

std::string KeyInfo::GetNspace() const {
  return JoinNamespace(splited_namespaces_and_key_.size() - 1);
}

size_t KeyInfo::GetNspaceSize() const {
  if (splited_namespaces_and_key_.empty()) {
    return 0;
  }

  return splited_namespaces_and_key_.size() - 1;
}

std::string KeyInfo::JoinNamespace(size_t pos) const {
  size_t ns_size = GetNspaceSize();
  if (ns_size > pos) {
    std::vector<std::string> copy;
    for (size_t i = 0; i <= pos; ++i) {
      copy.push_back(splited_namespaces_and_key_[i]);
    }
    return common::JoinString(copy, ns_separator_);
  }

  return std::string();
}

KeyInfo MakeKeyInfo(const core::key_t& key, const std::string& ns_separator) {
  KeyInfo::splited_namespaces_t tokens;
  common::Tokenize(key.ToString(), ns_separator, &tokens);
  return KeyInfo(tokens, ns_separator);
}

}  // namespace proxy
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::proxy::supportedViews v) {
  return fastonosql::proxy::supported_views_text[v];
}

bool ConvertFromString(const std::string& from, fastonosql::proxy::supportedViews* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < fastonosql::proxy::supported_views_text.size(); ++i) {
    if (from == fastonosql::proxy::supported_views_text[i]) {
      *out = static_cast<fastonosql::proxy::supportedViews>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

}  // namespace common
