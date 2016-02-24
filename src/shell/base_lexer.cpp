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

#include "shell/base_lexer.h"

#include "common/convert2string.h"

namespace fastonosql {

BaseQsciApi::BaseQsciApi(QsciLexer* lexer)
  : QsciAbstractAPIs(lexer), filtered_version_(UNDEFINED_SINCE) {
}

bool BaseQsciApi::canSkipCommand(const CommandInfo& info) const {
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

BaseQsciLexer::BaseQsciLexer(QObject* parent)
  : QsciLexerCustom(parent) {
}

QString BaseQsciLexer::description(int style) const {
  switch (style) {
    case Default:
      return "Default";
    case Command:
      return "Command";
    case HelpKeyword:
      return "HelpKeyword";
  }

  return QString(style);
}

QColor BaseQsciLexer::defaultColor(int style) const {
  switch (style) {
    case Default:
      return Qt::black;
    case Command:
      return Qt::red;
    case HelpKeyword:
      return Qt::red;
  }

  return Qt::black;
}

QString makeCallTip(const CommandInfo& info) {
  std::string since_str = convertVersionNumberToReadableString(info.since);
  QString qsince_str = common::convertFromString<QString>(since_str);
  return QString("Arguments: %1\nSummary: %2\nSince: %3\nExample: %4")
      .arg(common::convertFromString<QString>(info.params))
      .arg(common::convertFromString<QString>(info.summary))
      .arg(qsince_str)
      .arg(common::convertFromString<QString>(info.example));
}

}  // namespace fastonosql
