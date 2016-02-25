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

#include "shell/lmdb_lexer.h"

#include <vector>
#include <algorithm>

#include "core/lmdb/lmdb_raw.h"

namespace fastonosql {

LmdbApi::LmdbApi(QsciLexer *lexer)
  : BaseQsciApiCommandHolder(lmdb::lmdbCommands, lexer) {
}

LmdbLexer::LmdbLexer(QObject* parent)
  : BaseQsciLexer(parent) {
  setAPIs(new LmdbApi(this));
}

const char *LmdbLexer::language() const {
  return "LMDB";
}

const char* LmdbLexer::version() const {
  return lmdb::LmdbRaw::versionApi();
}

const char* LmdbLexer::basedOn() const {
  return "liblmdb";
}

std::vector<uint32_t> LmdbLexer::supportedVersions() const {
  std::vector<uint32_t> result;
  for (size_t i = 0; i < lmdb::lmdbCommands.size(); ++i) {
    CommandInfo cmd = lmdb::lmdbCommands[i];

    bool needed_insert = true;
    for (size_t j = 0; j < result.size(); ++j) {
      if (result[j] == cmd.since) {
        needed_insert = false;
        break;
      }
    }

    if (needed_insert) {
      result.push_back(cmd.since);
    }
  }

  std::sort(result.begin(), result.end());
  return result;
}

size_t LmdbLexer::commandsCount() const {
  return lmdb::lmdbCommands.size();
}

void LmdbLexer::styleText(int start, int end) {
  if (!editor()) {
    return;
  }

  char *data = new char[end - start + 1];
  editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, data);
  QString source(data);
  delete [] data;

  if (source.isEmpty()) {
    return;
  }

  paintCommands(source, start);

  int index = 0;
  int begin = 0;
  while ((begin = source.indexOf(help, index, Qt::CaseInsensitive)) != -1) {
    index = begin + help.length();

    startStyling(start + begin);
    setStyling(help.length(), HelpKeyword);
    startStyling(start + begin);
  }
}

void LmdbLexer::paintCommands(const QString& source, int start) {
  for (size_t i = 0; i < lmdb::lmdbCommands.size(); ++i) {
    CommandInfo cmd = lmdb::lmdbCommands[i];
    QString word = common::convertFromString<QString>(cmd.name);
    int index = 0;
    int begin = 0;
    while ((begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1) {
      index = begin + word.length();

      startStyling(start + begin);
      setStyling(word.length(), Command);
      startStyling(start + begin);
    }
  }
}

}  // namespace fastonosql
