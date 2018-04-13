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

#include "core/types.h"

#include <algorithm>

#include <common/string_util.h>

#include <common/convert2string.h>

namespace {

template <typename T>
std::string string_from_hex_impl(const T& value) {
  size_t len = value.size();
  if (len % 4 != 0) {
    return std::string();
  }

  std::string hex_digits;
  for (size_t i = 0; i < len; i += 4) {
    auto c1 = value[i];
    auto c2 = value[i + 1];
    if (c1 == '\\' && c2 == 'x') {
      hex_digits += value[i + 2];
      hex_digits += value[i + 3];
    } else {
      return std::string();
    }
  }

  return common::utils::hex::decode(hex_digits);
}

template <typename T>
std::string hex_string_impl(const T& value) {
  std::ostringstream wr;
  auto hexed = common::utils::hex::encode(value, true);
  for (size_t i = 0; i < hexed.size(); i += 2) {
    wr << "\\x";
    wr << hexed[i];
    wr << hexed[i + 1];
  }

  return wr.str();
}

}  // namespace

namespace fastonosql {
namespace core {

command_buffer_t StableCommand(const command_buffer_t& command) {
  if (command.empty()) {
    return command_buffer_t();
  }

  std::vector<command_buffer_t> tokens;
  size_t tok = common::Tokenize(command, " ", &tokens);
  command_buffer_t stabled_command;
  if (tok > 0) {
    command_buffer_t part = tokens[0];
    if (part.size() % 4 == 0 && part[0] == '\\' && (part[1] == 'x' || part[1] == 'X')) {
      stabled_command += "\"" + part + "\"";
    } else {
      stabled_command += part;
    }

    for (size_t i = 1; i < tok; ++i) {
      command_buffer_t part = tokens[i];
      stabled_command += " ";
      if (part.size() % 4 == 0 && part[0] == '\\' && (part[1] == 'x' || part[1] == 'X')) {
        stabled_command += "\"" + part + "\"";
      } else {
        stabled_command += part;
      }
    }
  }

  if (stabled_command[stabled_command.size() - 1] == '\r') {
    stabled_command.pop_back();
  }

  return stabled_command;
}

ReadableString::ReadableString() : data_(), type_(TEXT_DATA) {}

ReadableString::ReadableString(const readable_string_t& data) : data_(), type_(TEXT_DATA) {
  SetData(data);
}

ReadableString::DataType ReadableString::GetType() const {
  return type_;
}

readable_string_t ReadableString::GetData() const {
  return data_;
}

readable_string_t ReadableString::GetHumanReadable() const {
  if (type_ == BINARY_DATA) {
    return detail::hex_string(data_);
  }

  return data_;
}

readable_string_t ReadableString::GetForCommandLine() const {
  if (type_ == BINARY_DATA) {
    command_buffer_writer_t wr;
    wr << "\"" << detail::hex_string(data_) << "\"";
    return wr.str();
  }

  if (detail::is_json(data_)) {
    return data_;
  }

  if (detail::have_space(data_)) {
    return "\"" + data_ + "\"";
  }

  return data_;
}

void ReadableString::SetData(const readable_string_t& data) {
  data_ = data;
  type_ = detail::is_binary_data(data) ? BINARY_DATA : TEXT_DATA;
}

bool ReadableString::Equals(const ReadableString& other) const {
  return type_ == other.type_ && data_ == other.data_;
}

namespace detail {

std::string hex_string(const common::buffer_t& value) {
  return hex_string_impl(value);
}

std::string hex_string(const std::string& value) {
  return hex_string_impl(value);
}

std::string string_from_hex(const common::buffer_t& value) {
  return string_from_hex_impl(value);
}

std::string string_from_hex(const std::string& value) {
  return string_from_hex_impl(value);
}

std::string unicode_string(const std::string& value) {
  std::ostringstream wr;
  common::string16 s16 = common::ConvertToString16(value);
  std::string unicoded = common::utils::unicode::encode(s16, true);
  for (size_t i = 0; i < unicoded.size(); i += 4) {
    wr << "\\u";
    wr << unicoded[i];
    wr << unicoded[i + 1];
    wr << unicoded[i + 2];
    wr << unicoded[i + 3];
  }

  return wr.str();
}

std::string string_from_unicode(const std::string& value) {
  size_t len = value.size();
  if (len % 6 != 0) {
    return std::string();
  }

  std::string unicode_digits;
  for (size_t i = 0; i < len; i += 6) {
    auto c1 = value[i];
    auto c2 = value[i + 1];
    if (c1 == '\\' && c2 == 'u') {
      unicode_digits += value[i + 2];
      unicode_digits += value[i + 3];
      unicode_digits += value[i + 4];
      unicode_digits += value[i + 5];
    } else {
      return std::string();
    }
  }

  common::string16 s16 = common::utils::unicode::decode(unicode_digits);
  return common::ConvertToString(s16);
}

bool have_space(const std::string& data) {
  auto it = std::find_if(data.begin(), data.end(), [](char c) { return std::isspace(c); });
  return it != data.end();
}

bool is_binary_data(const command_buffer_t& data) {
  for (size_t i = 0; i < data.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(data[i]);
    if (c < ' ') {  // should be hexed symbol
      return true;
    }
  }
  return false;
}

bool is_json(const std::string& data) {
  if (data.empty()) {
    return false;
  }

  return (data[0] == '{' && data[data.size() - 1] == '}') || (data[0] == '[' && data[data.size() - 1] == ']');
}
}  // namespace detail

}  // namespace core
}  // namespace fastonosql
