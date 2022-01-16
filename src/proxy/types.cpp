/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "proxy/types.h"

#include <common/string_util.h>

#include <fastonosql/core/macros.h>

namespace fastonosql {
namespace proxy {

const std::vector<const char*> g_display_strategy_types = {"FULL_KEY", "KEY_NAME"};

const std::vector<const char*> g_supported_views_text = {"Tree", "Table", "Text"};

core::command_buffer_t StableCommand(core::command_buffer_t command) {
  if (!command.empty()) {
    if (command[command.size() - 1] == CARRIGE_RETURN_CHAR) {
      command.pop_back();
    }
  }

  return command;
}

common::Error ParseCommands(const core::command_buffer_t& cmd, std::vector<core::command_buffer_t>* cmds) {
  if (cmd.empty()) {
    return common::make_error("Empty command line.");
  }

  std::vector<core::command_buffer_t> commands;
  const size_t commands_count = common::Tokenize(cmd, {END_COMMAND_CHAR}, &commands);
  if (!commands_count) {
    return common::make_error("Invaid command line.");
  }

  std::vector<core::command_buffer_t> stable_commands;
  for (core::command_buffer_t input : commands) {
    const auto stable_input = StableCommand(input);
    if (stable_input.empty()) {
      continue;
    }
    stable_commands.push_back(stable_input);
  }

  *cmds = stable_commands;
  return common::Error();
}

}  // namespace proxy
}  // namespace fastonosql
