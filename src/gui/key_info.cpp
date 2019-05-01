/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "gui/key_info.h"

#include <common/string_util.h>

namespace fastonosql {
namespace gui {

KeyInfo::KeyInfo(const core::raw_key_t& key, const ns_separator_t& ns_separator)
    : key_(key), splited_namespaces_(), key_name_(), ns_separator_(ns_separator) {
  const core::readable_string_t rns = GEN_READABLE_STRING_SIZE(ns_separator.data(), ns_separator.size());
  size_t tokens = common::Tokenize(key, rns, &splited_namespaces_);
  if (tokens > 0) {
    key_name_ = splited_namespaces_.back();
    splited_namespaces_.pop_back();
  } else {
    DNOTREACHED() << "Invalid key: " << key;
  }
}

KeyInfo::string_t KeyInfo::keyName() const {
  return key_name_;
}

KeyInfo::string_t KeyInfo::key() const {
  return key_;
}

bool KeyInfo::hasNamespace() const {
  return splited_namespaces_.size() > 0;
}

KeyInfo::splited_namespaces_t KeyInfo::namespaces() const {
  return splited_namespaces_;
}

KeyInfo::ns_separator_t KeyInfo::nsSeparator() const {
  return ns_separator_;
}

}  // namespace gui
}  // namespace fastonosql
