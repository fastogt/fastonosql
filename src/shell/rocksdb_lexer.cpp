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

#include "shell/rocksdb_lexer.h"

#include "core/rocksdb/db_connection.h"

namespace fastonosql {
namespace shell {

RocksdbApi::RocksdbApi(QsciLexer* lexer)
  : BaseQsciApiCommandHolder(core::rocksdb::rocksdbCommands, lexer) {
}

RocksdbLexer::RocksdbLexer(QObject* parent)
  : BaseQsciLexerCommandHolder(core::rocksdb::rocksdbCommands, parent) {
  setAPIs(new RocksdbApi(this));
}

const char* RocksdbLexer::language() const {
  return "RocksDB";
}

const char* RocksdbLexer::version() const {
  return core::rocksdb::DBConnection::versionApi();
}

const char* RocksdbLexer::basedOn() const {
  return "rocksdb";
}

}  // namespace shell
}  // namespace fastonosql
