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

CommandHandler::CommandHandler(ICommandTranslator* translator) : translator_(translator) {}

common::Error CommandHandler::Execute(int argc, const char** argv, FastoObject* out) {
  const CommandInfo* info = nullptr;
  size_t off = 0;
  common::Error err = translator_->TestCommandLine(argc, argv, &info, &off);
  if (err && err->isError()) {
    return err;
  }

  int argc_to_call = argc - off;
  const char** argv_to_call = argv + off;
  const command_t* cmd = static_cast<const command_t*>(info);
  return cmd->Execute(this, argc_to_call, argv_to_call, out);
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
