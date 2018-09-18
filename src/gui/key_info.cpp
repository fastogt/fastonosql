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

#include "gui/key_info.h"

#include <common/string_util.h>

namespace fastonosql {
namespace gui {

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

std::string KeyInfo::GetNsSeparator() const {
  return ns_separator_;
}

}  // namespace gui
}  // namespace fastonosql
