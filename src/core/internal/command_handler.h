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

#include <memory>

#include "core/command_holder.h"  // for CommandHolder
#include "core/icommand_translator.h"

namespace fastonosql {
namespace core {
class FastoObject;
namespace internal {

class CommandHandler {
 public:
  typedef fastonosql::core::CommandHolder command_t;
  typedef std::vector<command_t> commands_t;

  explicit CommandHandler(ICommandTranslator* translator);  // take ownerships
  common::Error Execute(const command_buffer_t& command, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error Execute(commands_args_t argv, FastoObject* out) WARN_UNUSED_RESULT;

  translator_t GetTranslator() const { return translator_; }

 protected:
  template <typename T>
  std::shared_ptr<T> GetSpecificTranslator() const {
    return std::static_pointer_cast<T>(translator_);
  }

 private:
  translator_t translator_;
};

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
