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

#include "core/command_holder.h"

namespace {
auto count_space(const std::string& data) -> std::string::difference_type {
  return std::count_if(data.begin(), data.end(), [](char c) { return std::isspace(c); });
}
}  // namespace

namespace fastonosql {
namespace core {

common::Error TestArgsInRange(const CommandInfo& cmd, commands_args_t argv) {
  const size_t argc = argv.size();
  const uint16_t max = cmd.MaxArgumentsCount();
  const uint16_t min = cmd.MinArgumentsCount();
  if (argc > max || argc < min) {
    std::string buff =
        common::MemSPrintf("Invalid input argument for command: '%s', passed %d arguments, must be in range %u - %u.",
                           cmd.name, argc, min, max);
    return common::make_error(buff, common::ERROR_TYPE);
  }

  return common::Error();
}

common::Error TestArgsModule2Equal1(const CommandInfo& cmd, commands_args_t argv) {
  const size_t argc = argv.size();
  if (argc % 2 != 1) {
    std::string buff = common::MemSPrintf(
        "Invalid input argument for command: '%s', passed %d arguments, must be 1 by module 2.", cmd.name, argv.size());
    return common::make_error(buff, common::ERROR_TYPE);
  }

  return common::Error();
}

CommandHolder::CommandHolder(const std::string& name,
                             const std::string& params,
                             const std::string& summary,
                             uint32_t since,
                             const std::string& example,
                             uint8_t required_arguments_count,
                             uint8_t optional_arguments_count,
                             function_t func,
                             test_functions_t tests)
    : CommandInfo(name, params, summary, since, example, required_arguments_count, optional_arguments_count),
      func_(func),
      white_spaces_count_(count_space(name)),
      test_funcs_(tests) {}

bool CommandHolder::IsCommand(commands_args_t argv, size_t* offset) const {
  if (argv.empty()) {
    return false;
  }

  const size_t uargc = argv.size();
  if (uargc == white_spaces_count_) {
    return false;
  }

  CHECK(uargc > white_spaces_count_);
  std::vector<command_buffer_t> merged;
  for (size_t i = 0; i < white_spaces_count_ + 1; ++i) {
    merged.push_back(argv[i]);
  }

  command_buffer_t ws = common::JoinString(merged, " ");
  if (!IsEqualName(ws)) {
    return false;
  }

  if (offset) {
    *offset = white_spaces_count_ + 1;
  }
  return true;
}

common::Error CommandHolder::TestArgs(commands_args_t argv) const {
  const CommandInfo inf = *this;
  for (test_function_t func : test_funcs_) {
    common::Error err = func(inf, argv);
    if (err) {
      return err;
    }
  }

  return common::Error();
}

}  // namespace core
}  // namespace fastonosql
