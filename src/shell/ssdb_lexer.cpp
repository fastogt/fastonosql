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

#include "shell/ssdb_lexer.h"

#include "core/ssdb/db_connection.h"

namespace fastonosql {
namespace shell {

SsdbApi::SsdbApi(QsciLexer* lexer) : BaseQsciApiCommandHolder(core::ssdb::ssdbCommands, lexer) {}

SsdbLexer::SsdbLexer(QObject* parent)
    : BaseQsciLexerCommandHolder(core::ssdb::ssdbCommands, parent) {
  setAPIs(new SsdbApi(this));
}

const char* SsdbLexer::language() const {
  return "SSDB";
}

const char* SsdbLexer::version() const {
  return core::ssdb::DBConnection::versionApi();
}

const char* SsdbLexer::basedOn() const {
  return "ssdb-cli";
}

}  // namespace shell
}  // namespace fastonosql
