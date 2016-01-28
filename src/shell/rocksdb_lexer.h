/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "shell/base_lexer.h"

namespace fastonosql {

class RocksdbApi
  : public BaseQsciApi {
  Q_OBJECT
 public:
  explicit RocksdbApi(QsciLexer* lexer);

  virtual void updateAutoCompletionList(const QStringList& context, QStringList& list);
  virtual QStringList callTips(const QStringList& context, int commas,
                               QsciScintilla::CallTipsStyle style, QList<int>& shifts);
};

class RocksdbLexer
  : public BaseQsciLexer {
  Q_OBJECT
 public:
  explicit RocksdbLexer(QObject* parent = 0);

  virtual const char* language() const;
  virtual const char* version() const;
  virtual const char* basedOn() const;

  virtual std::vector<uint32_t> supportedVersions() const;
  virtual uint32_t commandsCount() const;

  virtual void styleText(int start, int end);

 private:
  void paintCommands(const QString& source, int start);
};

}
