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

#pragma once

#include <string>  // for string

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT

#include "core/command_holder.h"  // for CommandHolder

namespace fastonosql {
class FastoObject;
}  // lines 30-30

namespace fastonosql {
namespace core {
namespace internal {

class CommandHandler {
 public:
  typedef fastonosql::core::CommandHolder command_t;
  typedef std::vector<command_t> commands_t;

  explicit CommandHandler(const commands_t& commands);
  common::Error Execute(int argc, const char** argv, FastoObject* out) WARN_UNUSED_RESULT;

  static common::Error NotSupported(const std::string& cmd);
  static common::Error UnknownSequence(int argc, const char** argv);

 protected:
  common::Error FindCommand(int argc,
                            const char** argv,
                            const command_t** cmdout) const WARN_UNUSED_RESULT;

 private:
  const commands_t commands_;
};
}  // namespace internal
}  // namespace core
}  // namespace fastonosql
