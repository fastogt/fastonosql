/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "gui/db/rocksdb/lexer.h"

namespace fastonosql {
namespace gui {
namespace rocksdb {

RocksDBApi::RocksDBApi(Lexer* lexer) : BaseCommandsQsciApi(lexer) {}

Lexer::Lexer(QObject* parent) : BaseCommandsQsciLexer(rocksdb_trait_t::GetCommands(), parent) {
  setAPIs(new RocksDBApi(this));
}

const char* Lexer::language() const {
  return rocksdb_trait_t::GetDBName();
}

const char* Lexer::version() const {
  return rocksdb_trait_t::GetVersionApi();
}

const char* Lexer::basedOn() const {
  return rocksdb_trait_t::GetBasedOn();
}

}  // namespace rocksdb
}  // namespace gui
}  // namespace fastonosql
