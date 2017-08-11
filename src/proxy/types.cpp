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

#include <stddef.h>  // for size_t

#include <common/convert2string.h>
#include <common/macros.h>  // for SIZEOFMASS
#include <common/string_util.h>

namespace {
/* Helper function for sdssplitargs() that returns non zero if 'c'
 * is a valid hex digit. */
bool is_hex_digit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int hex_digit_to_int(char c) {
  switch (c) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'a':
    case 'A':
      return 10;
    case 'b':
    case 'B':
      return 11;
    case 'c':
    case 'C':
      return 12;
    case 'd':
    case 'D':
      return 13;
    case 'e':
    case 'E':
      return 14;
    case 'f':
    case 'F':
      return 15;
    default:
      return 0;
  }
}

}  // namespace

namespace fastonosql {
namespace {
size_t extract_binary(const core::command_buffer_t& command) {
  if (command.empty()) {
    return 0;
  }

  size_t i = 0;
  for (size_t j = 0; j < command.size(); ++j) {
    core::command_buffer_char_t c = command[j];
    bool is_last = j == command.size() - 1;
    if (c == '\\' && !is_last && command[j + 1] == 'x') {
      bool is_pre_last = j == command.size() - 2;
      bool is_pre_pre_last = j == command.size() - 3;
      if (!is_pre_last && !is_pre_pre_last) {
        core::command_buffer_char_t h1 = command[j + 2];
        core::command_buffer_char_t h2 = command[j + 3];
        if (is_hex_digit(h1) && is_hex_digit(h2)) {
          i += 4;
          j += 3;
        }
      }
    }
  }

  return i;
}
}  // namespace
namespace proxy {

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

core::command_buffer_t StableCommand(core::command_buffer_t command) {
  if (command.empty()) {
    return core::command_buffer_t();
  }

  core::command_buffer_t stabled_command;
  for (size_t i = 0; i < command.size(); ++i) {
    core::command_buffer_char_t c = command[i];
    bool is_last_first = i == command.size() - 1;
    if (c == '\\' && !is_last_first && command[i + 1] == 'x') {
      size_t pos = extract_binary(command.c_str() + i);
      if (pos != 0) {
        stabled_command += "\"" + command.substr(i, pos) + "\"";
        i += pos - 1;
        continue;
      }
    }

    stabled_command += c;
  }

  return stabled_command;
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

  for (size_t i = 0; i < SIZEOFMASS(fastonosql::proxy::supported_views_text); ++i) {
    if (from == fastonosql::proxy::supported_views_text[i]) {
      *out = static_cast<fastonosql::proxy::supportedViews>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

}  // namespace common
