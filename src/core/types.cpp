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

#include "core/types.h"

#include <common/string_util.h>

#include <string>  // for string

namespace fastonosql {
namespace core {

IStateField::~IStateField() {}

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

}  // namespace core
}  // namespace fastonosql
