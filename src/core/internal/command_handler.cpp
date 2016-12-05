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

#include "core/internal/command_handler.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint16_t

#include <string>  // for string

#include <common/value.h>    // for ErrorValue, etc
#include <common/sprintf.h>  // for MemSPrintf

namespace fastonosql {
namespace core {
namespace internal {

CommandHandler::CommandHandler(const commands_t& commands) : commands_(commands) {}

common::Error CommandHandler::Execute(int argc, const char** argv, FastoObject* out) {
  for (auto cmd : commands_) {
    size_t off = 0;
    if (cmd.IsCommand(argc, argv, &off)) {
      int argc_to_call = argc - off;
      const char** argv_to_call = argv + off;
      uint16_t max = cmd.MaxArgumentsCount();
      uint16_t min = cmd.MinArgumentsCount();
      if (argc_to_call > max || argc_to_call < min) {
        std::string buff = common::MemSPrintf(
            "Invalid input argument for command: '%s', passed %d, must be in range %d - %d.",
            cmd.name, argc_to_call, max, min);
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }

      return cmd.Execute(this, argc_to_call, argv_to_call, out);
    }
  }

  return UnknownSequence(argc, argv);
}

common::Error CommandHandler::FindCommand(int argc,
                                          const char** argv,
                                          const command_t** cmdout) const {
  if (!cmdout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*cmdout == NULL);
  for (size_t i = 0; i < commands_.size(); ++i) {
    size_t off = 0;
    if (commands_[i].IsCommand(argc, argv, &off)) {
      *cmdout = &commands_[i];
      return common::Error();
    }
  }

  return UnknownSequence(argc, argv);
}

common::Error CommandHandler::NotSupported(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s.", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error CommandHandler::UnknownSequence(int argc, const char** argv) {
  std::string result;
  for (int i = 0; i < argc; ++i) {
    result += argv[i];
    if (i != argc - 1) {
      result += " ";
    }
  }
  std::string buff = common::MemSPrintf("Unknown sequence: '%s'.", result);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}
}  // namespace internal
}  // namespace core
}  // namespace fastonosql
