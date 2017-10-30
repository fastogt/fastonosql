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

#include "gui/shell/base_lexer.h"

#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/sprintf.h>

namespace fastonosql {
namespace gui {
namespace {
BaseCommandsQsciLexer::validated_commands_t MakeValidatedCommands(const std::vector<core::CommandHolder>& commands) {
  BaseCommandsQsciLexer::validated_commands_t res;
  for (size_t i = 0; i < commands.size(); ++i) {
    res.push_back(commands[i]);
  }
  return res;
}
}

BaseQsciApi::BaseQsciApi(QsciLexer* lexer) : QsciAbstractAPIs(lexer), filtered_version_(UNDEFINED_SINCE) {}

bool BaseQsciApi::canSkipCommand(const core::CommandInfo& info) const {
  if (filtered_version_ == UNDEFINED_SINCE) {
    return false;
  }

  if (info.since == UNDEFINED_SINCE) {
    return false;
  }

  return info.since > filtered_version_;
}

void BaseQsciApi::setFilteredVersion(uint32_t version) {
  filtered_version_ = version;
}

BaseCommandsQsciApi::BaseCommandsQsciApi(BaseCommandsQsciLexer* lexer) : BaseQsciApi(lexer) {}

void BaseCommandsQsciApi::updateAutoCompletionList(const QStringList& context, QStringList& list) {
  BaseCommandsQsciLexer* lex = static_cast<BaseCommandsQsciLexer*>(lexer());
  auto commands = lex->commands();
  for (auto it = context.begin(); it != context.end(); ++it) {
    QString val = *it;
    for (size_t i = 0; i < commands.size(); ++i) {
      core::CommandInfo cmd = commands[i];
      if (canSkipCommand(cmd)) {
        continue;
      }

      QString jval;
      common::ConvertFromString(cmd.name, &jval);
      if (jval.startsWith(val, Qt::CaseInsensitive)) {
        list.append(jval + "?1");
      }
    }
  }
}

QStringList BaseCommandsQsciApi::callTips(const QStringList& context,
                                          int commas,
                                          QsciScintilla::CallTipsStyle style,
                                          QList<int>& shifts) {
  UNUSED(commas);
  UNUSED(style);
  UNUSED(shifts);
  BaseCommandsQsciLexer* lex = static_cast<BaseCommandsQsciLexer*>(lexer());
  auto commands = lex->commands();
  for (auto it = context.begin(); it != context.end(); ++it) {
    QString val = *it;
    for (size_t i = 0; i < commands.size(); ++i) {
      core::CommandInfo cmd = commands[i];

      QString jval;
      common::ConvertFromString(cmd.name, &jval);
      if (QString::compare(jval, val, Qt::CaseInsensitive) == 0) {
        return QStringList() << makeCallTip(cmd);
      }
    }
  }

  return QStringList();
}

BaseQsciLexer::BaseQsciLexer(QObject* parent) : QsciLexerCustom(parent) {}

QString BaseQsciLexer::description(int style) const {
  switch (style) {
    case Default:
      return "Default";
    case Command:
      return "Native command";
    case ExCommand:
      return "Extended command";
  }

  return QString(style);
}

QColor BaseQsciLexer::defaultColor(int style) const {
  switch (style) {
    case Default:
      return Qt::black;
    case Command:
      return Qt::red;
    case ExCommand:
      return Qt::magenta;
  }

  return Qt::black;
}

BaseQsciApi* BaseQsciLexer::apis() const {
  BaseQsciApi* api = dynamic_cast<BaseQsciApi*>(QsciLexerCustom::apis());  // +
  CHECK(api);
  return api;
}

BaseCommandsQsciLexer::BaseCommandsQsciLexer(const std::vector<core::CommandHolder>& commands, QObject* parent)
    : BaseQsciLexer(parent), commands_(MakeValidatedCommands(commands)) {}

std::vector<uint32_t> BaseCommandsQsciLexer::supportedVersions() const {
  std::vector<uint32_t> result;
  for (size_t i = 0; i < commands_.size(); ++i) {
    core::CommandInfo cmd = commands_[i];

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

const BaseCommandsQsciLexer::validated_commands_t& BaseCommandsQsciLexer::commands() const {
  return commands_;
}

size_t BaseCommandsQsciLexer::commandsCount() const {
  return commands_.size();
}

void BaseCommandsQsciLexer::styleText(int start, int end) {
  if (!editor()) {
    return;
  }

  char* data = new char[end - start + 1];
  editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, data);
  QString source(data);
  delete[] data;

  if (source.isEmpty()) {
    return;
  }

  paintCommands(source, start);
}

void BaseCommandsQsciLexer::paintCommands(const QString& source, int start) {
  for (size_t i = 0; i < commands_.size(); ++i) {
    core::CommandInfo cmd = commands_[i];
    QString word;
    if (common::ConvertFromString(cmd.name, &word)) {
      int index = 0;
      int begin = 0;

      int st = cmd.type == core::CommandInfo::Native ? Command : ExCommand;
      while ((begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1) {
        index = begin + word.length();

        startStyling(start + begin);
        setStyling(word.length(), st);
        startStyling(start + begin);
      }
    }
  }
}

QString makeCallTip(const core::CommandInfo& info) {
  std::string since_str = core::ConvertVersionNumberToReadableString(info.since);

  std::string res = common::MemSPrintf(
      "Arguments: %s\nSummary: %s\nSince: "
      "%s\nExample: %s",
      info.params, info.summary, since_str, info.example);
  QString qres;
  common::ConvertFromString(res, &qres);
  return qres;
}

}  // namespace gui
}  // namespace fastonosql
