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

KeyInfo::KeyInfo(const core::key_t& key, const string_t &ns_separator)
    : key_(key), splited_namespaces_(), key_name_(), ns_separator_(ns_separator) {
  common::Tokenize(key.GetHumanReadable(), ns_separator, &splited_namespaces_);
  key_name_ = splited_namespaces_.back();
  splited_namespaces_.pop_back();
}

KeyInfo::string_t KeyInfo::keyName() const {
  return key_name_;
}

KeyInfo::string_t KeyInfo::key() const {
  return key_.GetHumanReadable();
}

bool KeyInfo::hasNamespace() const {
  size_t ns_size = nsSplitedSize();
  return ns_size > 0;
}

size_t KeyInfo::nsSplitedSize() const {
  return splited_namespaces_.size();
}

KeyInfo::splited_namespaces_t KeyInfo::namespaces() const {
  return splited_namespaces_;
}

KeyInfo::string_t KeyInfo::nsSeparator() const {
  return ns_separator_;
}

}  // namespace gui
}  // namespace fastonosql
