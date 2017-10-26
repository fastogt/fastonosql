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

#pragma once

#include <functional>
#include <vector>

#include <common/error.h>  // for Error

#include "core/command_info.h"  // for CommandInfo
#include "core/types.h"

namespace fastonosql {
namespace core {
class FastoObject;
namespace internal {
class CommandHandler;
}

common::Error TestArgsInRange(const CommandInfo& cmd, commands_args_t argv);
common::Error TestArgsModule2Equal1(const CommandInfo& cmd, commands_args_t argv);

class CommandHolder : public CommandInfo {
 public:
  friend class internal::CommandHandler;

  typedef internal::CommandHandler command_handler_t;
  typedef std::function<common::Error(command_handler_t*, commands_args_t, FastoObject*)> function_t;
  typedef std::function<common::Error(const CommandInfo&, commands_args_t)> test_function_t;
  typedef std::vector<test_function_t> test_functions_t;

  CommandHolder(const std::string& name,
                const std::string& params,
                const std::string& summary,
                uint32_t since,
                const std::string& example,
                uint8_t required_arguments_count,
                uint8_t optional_arguments_count,
                Type type,
                function_t func,
                test_functions_t tests = {&TestArgsInRange});

  bool IsCommand(commands_args_t argv, size_t* offset) const;
  bool IsEqualFirstName(const std::string& cmd_first_name) const;

  common::Error TestArgs(commands_args_t argv) const WARN_UNUSED_RESULT;

 private:
  const function_t func_;
  const size_t white_spaces_count_;
  const std::vector<test_function_t> test_funcs_;
};

}  // namespace core
}  // namespace fastonosql
