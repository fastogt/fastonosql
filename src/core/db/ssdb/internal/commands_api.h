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
namespace ssdb {

class DBConnection;
struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScanSSDB(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Auth(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Setx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Rscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiDel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hgetall(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hincr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hrscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiHget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiHset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zincr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiZget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiZset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MultiZdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Qpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Qpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Qslice(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Qclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DBsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
};

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
