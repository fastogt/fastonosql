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

#include "gui/db/lmdb/lexer.h"

#include "core/db/lmdb/db_connection.h"

namespace fastonosql {
namespace gui {
namespace lmdb {

LmdbApi::LmdbApi(QsciLexer* lexer) : BaseQsciApiCommandHolder(core::lmdb::DBConnection::Commands(), lexer) {}

Lexer::Lexer(QObject* parent) : BaseQsciLexerCommandHolder(core::lmdb::DBConnection::Commands(), parent) {
  setAPIs(new LmdbApi(this));
}

const char* Lexer::language() const {
  return core::lmdb::DBConnection::GetConnectionTypeName();
}

const char* Lexer::version() const {
  return core::lmdb::DBConnection::VersionApi();
}

const char* Lexer::basedOn() const {
  return core::lmdb::DBConnection::BasedOn();
}

}  // namespace lmdb
}  // namespace gui
}  // namespace fastonosql
