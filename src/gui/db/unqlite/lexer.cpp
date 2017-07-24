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

#include "gui/db/unqlite/lexer.h"

#include "core/db/unqlite/db_connection.h"

namespace fastonosql {
namespace gui {
namespace unqlite {

UnqliteApi::UnqliteApi(QsciLexer* lexer)
    : BaseQsciApiCommandHolder(core::unqlite::DBConnection::Commands(), lexer) {}

Lexer::Lexer(QObject* parent)
    : BaseQsciLexerCommandHolder(core::unqlite::DBConnection::Commands(), parent) {
  setAPIs(new UnqliteApi(this));
}

const char* Lexer::language() const {
  return core::unqlite::DBConnection::GetConnectionTypeName();
}

const char* Lexer::version() const {
  return core::unqlite::DBConnection::VersionApi();
}

const char* Lexer::basedOn() const {
  return core::unqlite::DBConnection::BasedOn();
}

}  // namespace unqlite
}  // namespace gui
}  // namespace fastonosql
