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

#include "core/internal/commands_api.h"

namespace fastonosql {
namespace core {
namespace memcached {

class DBConnection;
struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Version(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Add(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Replace(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Append(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Prepend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Decr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
};

// TODO: cas command implementation
extern const internal::ConstantCommandsArray g_commands;

extern const internal::ConstantCommandsArray g_extended_commands;

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
