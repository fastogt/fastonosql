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

#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t
#include <vector>    // for vector

#include <Qsci/qsciabstractapis.h>
#include <Qsci/qscilexercustom.h>

namespace fastonosql {
namespace core {
class CommandHolder;
}
}
namespace fastonosql {
namespace core {
struct CommandInfo;
}
}

namespace fastonosql {
namespace shell {

class BaseQsciApi : public QsciAbstractAPIs {
  Q_OBJECT
 public:
  explicit BaseQsciApi(QsciLexer* lexer);
  void setFilteredVersion(uint32_t version);

 protected:
  bool canSkipCommand(const core::CommandInfo& info) const;

 private:
  uint32_t filtered_version_;
};

class BaseQsciApiCommandHolder : public BaseQsciApi {
  Q_OBJECT
 public:
  virtual void updateAutoCompletionList(const QStringList& context, QStringList& list);
  virtual QStringList callTips(const QStringList& context,
                               int commas,
                               QsciScintilla::CallTipsStyle style,
                               QList<int>& shifts);

 protected:
  BaseQsciApiCommandHolder(const std::vector<core::CommandHolder>& commands, QsciLexer* lexer);

 private:
  const std::vector<core::CommandHolder>& commands_;
};

class BaseQsciLexer : public QsciLexerCustom {
  Q_OBJECT
 public:
  enum { Default = 0, Command = 1 };

  virtual const char* version() const = 0;
  virtual const char* basedOn() const = 0;

  virtual std::vector<uint32_t> supportedVersions() const = 0;
  virtual size_t commandsCount() const = 0;

  virtual QString description(int style) const;
  virtual QColor defaultColor(int style) const;

  BaseQsciApi* apis() const;

 protected:
  explicit BaseQsciLexer(QObject* parent = 0);
};

class BaseQsciLexerCommandHolder : public BaseQsciLexer {
  Q_OBJECT
 public:
  virtual std::vector<uint32_t> supportedVersions() const;
  virtual size_t commandsCount() const;

 protected:
  explicit BaseQsciLexerCommandHolder(const std::vector<core::CommandHolder>& commands,
                                      QObject* parent = 0);

 private:
  virtual void styleText(int start, int end);
  void paintCommands(const QString& source, int start);

  const std::vector<core::CommandHolder>& commands_;
};

QString makeCallTip(const core::CommandInfo& info);

}  // namespace shell
}  // namespace fastonosql
