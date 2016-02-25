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

#include "shell/memcached_lexer.h"

#include <vector>
#include <algorithm>

#include "core/memcached/memcached_raw.h"

namespace fastonosql {

MemcachedApi::MemcachedApi(QsciLexer *lexer)
  : BaseQsciApiCommandHolder(memcached::memcachedCommands, lexer) {
}

MemcachedLexer::MemcachedLexer(QObject* parent)
  : BaseQsciLexer(parent) {
  setAPIs(new MemcachedApi(this));
}

const char *MemcachedLexer::language() const {
  return "Memcached";
}

const char* MemcachedLexer::version() const {
  return memcached::MemcachedRaw::versionApi();
}

const char* MemcachedLexer::basedOn() const {
  return "libmemcached";
}

std::vector<uint32_t> MemcachedLexer::supportedVersions() const {
  std::vector<uint32_t> result;
  for (size_t i = 0; i < memcached::memcachedCommands.size(); ++i) {
    CommandInfo cmd = memcached::memcachedCommands[i];

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

size_t MemcachedLexer::commandsCount() const {
  return memcached::memcachedCommands.size();
}

void MemcachedLexer::styleText(int start, int end) {
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

void MemcachedLexer::paintCommands(const QString& source, int start) {
  for (size_t i = 0; i < memcached::memcachedCommands.size(); ++i) {
    CommandInfo cmd = memcached::memcachedCommands[i];
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
