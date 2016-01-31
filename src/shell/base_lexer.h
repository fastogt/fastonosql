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

#include <Qsci/qsciabstractapis.h>
#include <Qsci/qscilexercustom.h>

#include "core/types.h"

namespace fastonosql {

class BaseQsciApi
  : public QsciAbstractAPIs {
  Q_OBJECT
 public:
  explicit BaseQsciApi(QsciLexer* lexer);
  void setFilteredVersion(uint32_t version);

 protected:
  bool canSkipCommand(const CommandInfo& info) const;

 private:
  uint32_t filtered_version_;
};

class BaseQsciLexer
  : public QsciLexerCustom {
  Q_OBJECT
 public:
  enum
  {
    Default = 0,
    Command = 1,
    HelpKeyword
  };

  virtual const char* version() const = 0;
  virtual const char* basedOn() const = 0;

  virtual std::vector<uint32_t> supportedVersions() const = 0;
  virtual uint32_t commandsCount() const = 0;

  virtual QString description(int style) const;
  virtual QColor defaultColor(int style) const;

 protected:
  explicit BaseQsciLexer(QObject* parent = 0);
};

QString makeCallTip(const CommandInfo& info);

}
