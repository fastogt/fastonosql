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

#include "gui/base_lexer.h"

namespace fastonosql {
namespace gui {
namespace leveldb {

class LeveldbApi : public BaseQsciApiCommandHolder {
  Q_OBJECT
 public:
  explicit LeveldbApi(QsciLexer* lexer);
};

class Lexer : public BaseQsciLexerCommandHolder {
  Q_OBJECT
 public:
  explicit Lexer(QObject* parent = 0);

  virtual const char* language() const override;
  virtual const char* version() const override;
  virtual const char* basedOn() const override;
};

}  // namespace leveldb
}  // namespace gui
}  // namespace fastonosql
