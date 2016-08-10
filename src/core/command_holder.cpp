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

#include "core/command_holder.h"

#include <string>                       // for string

#include "common/string_util.h"         // for FullEqualsASCII

namespace fastonosql {
namespace core {

CommandHolder::CommandHolder(const std::string& name, const std::string& params,
                             const std::string& summary, uint32_t since,
                             const std::string& example, uint8_t required_arguments_count,
                             uint8_t optional_arguments_count, function_t func)
  : CommandInfo(name, params, summary, since, example,
                required_arguments_count, optional_arguments_count), func_(func){
}

bool CommandHolder::isCommand(const std::string& cmd) {
  return common::FullEqualsASCII(cmd, name, false);
}

common::Error CommandHolder::execute(CommandHandler *handler, int argc, char** argv, FastoObject* out) {
  return func_(handler, argc, argv, out);
}

}  // namespace core
}  // namespace fastonosql
