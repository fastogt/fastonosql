/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <Qsci/qsciabstractapis.h>
#include <Qsci/qscilexercustom.h>

#include <fastonosql/core/command_holder.h>

namespace fastonosql {
namespace gui {

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

class BaseQsciLexer : public QsciLexerCustom {
  Q_OBJECT

 public:
  enum { Default = 0, Command = 1, ExCommand = 2 };

  const char* language() const override = 0;
  virtual const char* version() const = 0;
  virtual const char* basedOn() const = 0;

  virtual std::vector<uint32_t> supportedVersions() const = 0;
  virtual size_t commandsCount() const = 0;

  QString description(int style) const override;
  QColor defaultColor(int style) const override;

  BaseQsciApi* apis() const;

 protected:
  explicit BaseQsciLexer(QObject* parent = Q_NULLPTR);
};

class BaseCommandsQsciLexer : public BaseQsciLexer {
  Q_OBJECT

 public:
  typedef std::vector<core::CommandInfo> validated_commands_t;

  const char* language() const override = 0;
  const char* version() const override = 0;
  const char* basedOn() const override = 0;

  std::vector<uint32_t> supportedVersions() const override;
  size_t commandsCount() const override;
  const validated_commands_t& commands() const;

 protected:
  explicit BaseCommandsQsciLexer(const std::vector<core::CommandHolder>& commands, QObject* parent = Q_NULLPTR);

 private:
  void styleText(int start, int end) override;
  void paintCommands(const QString& source, int start);

  const validated_commands_t commands_;
};

class BaseCommandsQsciApi : public BaseQsciApi {
  Q_OBJECT

 public:
  void updateAutoCompletionList(const QStringList& context, QStringList& list) override;
  QStringList callTips(const QStringList& context,
                       int commas,
                       QsciScintilla::CallTipsStyle style,
                       QList<int>& shifts) override;

 protected:
  explicit BaseCommandsQsciApi(BaseCommandsQsciLexer* lexer);
};

QString makeCallTip(const core::CommandInfo& info);

}  // namespace gui
}  // namespace fastonosql
