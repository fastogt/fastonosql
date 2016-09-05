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

#include "shell/redis_lexer.h"

#include "core/redis/db_connection.h"  // for redisCommands, DBConnection

namespace fastonosql {
namespace shell {

RedisApi::RedisApi(QsciLexer* lexer)
    : BaseQsciApiCommandHolder(core::redis::redisCommands, lexer) {}

RedisLexer::RedisLexer(QObject* parent)
    : BaseQsciLexerCommandHolder(core::redis::redisCommands, parent) {
  setAPIs(new RedisApi(this));
}

const char* RedisLexer::language() const {
  return "Redis";
}

const char* RedisLexer::version() const {
  return core::redis::DBConnection::versionApi();
}

const char* RedisLexer::basedOn() const {
  return "hiredis";
}

}  // namespace shell
}  // namespace fastonosql
