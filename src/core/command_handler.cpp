/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "core/command_handler.h"

#include <vector>
#include <string>

#include "common/string_util.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace core {

CommandInfo::CommandInfo(const std::string& name, const std::string& params,
                         const std::string& summary, uint32_t since,
                         const std::string& example, uint8_t required_arguments_count,
                         uint8_t optional_arguments_count)
  : name(name), params(params), summary(summary), since(since), example(example),
    required_arguments_count(required_arguments_count),
    optional_arguments_count(optional_arguments_count) {
}

uint16_t CommandInfo::maxArgumentsCount() const {
  return required_arguments_count + optional_arguments_count;
}

uint8_t CommandInfo::minArgumentsCount() const {
  return required_arguments_count;
}

CommandHolder::CommandHolder(const std::string& name, const std::string& params,
                             const std::string& summary, uint32_t since,
                             const std::string& example, uint8_t required_arguments_count,
                             uint8_t optional_arguments_count, function_type func)
  : CommandInfo(name, params, summary, since, example,
                required_arguments_count, optional_arguments_count), func_(func){
}

bool CommandHolder::isCommand(const std::string& cmd) {
  return FullEqualsASCII(cmd, name, false);
}

common::Error CommandHolder::execute(CommandHandler *handler, int argc, char** argv, FastoObject* out) {
  return func_(handler, argc, argv, out);
}

CommandHandler::CommandHandler(const std::vector<commands_t> &commands)
  : commands_(commands) {
}

common::Error CommandHandler::execute(int argc, char** argv, FastoObject* out) {
  char* input_cmd = argv[0];
  for(size_t i = 0; i < commands_.size(); ++i) {
    commands_t cmd = commands_[i];
    if (cmd.isCommand(input_cmd)) {
      int argc_to_call = argc - 1;
      char** argv_to_call = argv + 1;
      if (argc_to_call > cmd.maxArgumentsCount() || argc_to_call < cmd.minArgumentsCount()) {
        std::string buff = common::MemSPrintf("Invalid input argument for command: %s", input_cmd);
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }

      return cmd.execute(this, argc_to_call, argv_to_call, out);
    }
  }

  return notSupported(input_cmd);
}

common::Error CommandHandler::notSupported(const char* cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

std::string convertVersionNumberToReadableString(uint32_t version) {
  if (version != UNDEFINED_SINCE) {
    return common::convertVersionNumberToString(version);
  }

  return UNDEFINED_SINCE_STR;
}

}  // namespace core
}  // namespace fastonosql
