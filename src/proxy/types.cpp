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

KeyInfo::KeyInfo(const core::key_t& key, std::string ns_separator)
    : key_(key), splited_namespaces_(), key_name_(), ns_separator_(ns_separator) {
  common::Tokenize(key.GetHumanReadable(), ns_separator, &splited_namespaces_);
  key_name_ = splited_namespaces_.back();
  splited_namespaces_.pop_back();
}

std::string KeyInfo::GetKeyName() const {
  return key_name_;
}

std::string KeyInfo::GetKey() const {
  return key_.GetHumanReadable();
}

bool KeyInfo::HasNamespace() const {
  size_t ns_size = GetNspaceSize();
  return ns_size > 0;
}

size_t KeyInfo::GetNspaceSize() const {
  return splited_namespaces_.size();
}

KeyInfo::splited_namespaces_t KeyInfo::GetNamespaces() const {
  return splited_namespaces_;
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
