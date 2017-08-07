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

#include "core/internal/command_handler.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint16_t

#include <string>  // for string

extern "C" {
#include "sds.h"
}

#include <common/sprintf.h>  // for MemSPrintf
#include <common/utils.h>
#include <common/value.h>  // for ErrorValue, etc

namespace fastonosql {
namespace core {
namespace internal {

CommandHandler::CommandHandler(ICommandTranslator* translator) : translator_(translator) {}

common::Error CommandHandler::Execute(const std::string& command, FastoObject* out) {
  const char* ccommand = common::utils::c_strornull(command);
  if (!ccommand) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  int argc;
  sds* argv = sdssplitargslong(ccommand, &argc);
  if (!argv) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  const char** exec_argv = const_cast<const char**>(argv);
  common::Error err = Execute(argc, exec_argv, out);
  sdsfreesplitres(argv, argc);
  return err;
}

common::Error CommandHandler::Execute(int argc, const char** argv, FastoObject* out) {
  const command_t* cmd = nullptr;
  size_t off = 0;
  common::Error err = translator_->TestCommandLineArgs(argc, argv, &cmd, &off);
  if (err && err->IsError()) {
    return err;
  }

  int argc_to_call = argc - off;
  const char** argv_to_call = argv + off;
  return cmd->func_(this, argc_to_call, argv_to_call, out);
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
