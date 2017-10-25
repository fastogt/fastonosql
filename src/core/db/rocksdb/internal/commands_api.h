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

#include "core/internal/commands_api.h"  // for ApiTraits

namespace fastonosql {
namespace core {
namespace rocksdb {

class DBConnection;
struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Merge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
};

extern const internal::ConstantCommandsArray g_commands;

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
