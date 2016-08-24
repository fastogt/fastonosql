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

#include <stddef.h>                     // for size_t

#include <string>                       // for string
#include <vector>                       // for vector

#include "common/sprintf.h"             // for MemSPrintf
#include "common/value.h"               // for ErrorValue, etc

namespace fastonosql {
namespace core {

CommandHandler::CommandHandler(const std::vector<commands_t> &commands)
  : commands_(commands) {
}

common::Error CommandHandler::execute(int argc, char** argv, FastoObject* out) {
  for(size_t i = 0; i < commands_.size(); ++i) {
    commands_t cmd = commands_[i];
    size_t off = cmd.commandOffset(argc, argv);
    if (off) {
      int argc_to_call = argc - off;
      char** argv_to_call = argv + off;
      if (argc_to_call > cmd.maxArgumentsCount() || argc_to_call < cmd.minArgumentsCount()) {
        std::string buff = common::MemSPrintf("Invalid input argument for command: %s", argv[0]);
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }

      return cmd.execute(this, argc_to_call, argv_to_call, out);
    }
  }

  return notSupported(argv[0]);
}

common::Error CommandHandler::notSupported(const char* cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

}  // namespace core
}  // namespace fastonosql
