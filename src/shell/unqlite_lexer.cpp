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

#include "shell/unqlite_lexer.h"

#include "core/unqlite/unqlite_driver.h"

namespace {
  const QString help("help");
}

namespace fastonosql {

UnqliteApi::UnqliteApi(QsciLexer *lexer)
  : BaseQsciApi(lexer) {
}

void UnqliteApi::updateAutoCompletionList(const QStringList& context, QStringList& list) {
  for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
    QString val = *it;
    for(int i = 0; i < SIZEOFMASS(unqliteCommands); ++i){
      CommandInfo cmd = unqliteCommands[i];
      if(canSkipCommand(cmd)){
        continue;
      }

      QString jval = common::convertFromString<QString>(cmd.name_);
      if(jval.startsWith(val, Qt::CaseInsensitive)){
        list.append(jval + "?1");
      }
    }

    if(help.startsWith(val, Qt::CaseInsensitive)){
      list.append(help + "?2");
    }
  }
}

QStringList UnqliteApi::callTips(const QStringList& context, int commas,
                                 QsciScintilla::CallTipsStyle style, QList<int>& shifts) {
  for(QStringList::const_iterator it = context.begin(); it != context.end() - 1; ++it){
      QString val = *it;
      for(int i = 0; i < SIZEOFMASS(unqliteCommands); ++i){
          CommandInfo cmd = unqliteCommands[i];

          QString jval = common::convertFromString<QString>(cmd.name_);
          if(QString::compare(jval, val, Qt::CaseInsensitive) == 0){
              return QStringList() << makeCallTip(cmd);
          }
      }
  }

  return QStringList();
}

UnqliteLexer::UnqliteLexer(QObject* parent)
    : BaseQsciLexer(parent) {
  setAPIs(new UnqliteApi(this));
}

const char *UnqliteLexer::language() const {
  return "Unqlite";
}

const char* UnqliteLexer::version() const {
  return UnqliteDriver::versionApi();
}

const char* UnqliteLexer::basedOn() const {
  return "unqlite";
}

std::vector<uint32_t> UnqliteLexer::supportedVersions() const {
  std::vector<uint32_t> result;
  for(int i = 0; i < SIZEOFMASS(unqliteCommands); ++i){
    CommandInfo cmd = unqliteCommands[i];

    bool needed_insert = true;
    for(int j = 0; j < result.size(); ++j){
      if(result[j] == cmd.since_){
        needed_insert = false;
        break;
      }
    }

    if(needed_insert){
      result.push_back(cmd.since_);
    }
  }

  std::sort(result.begin(), result.end());

  return result;
}

uint32_t UnqliteLexer::commandsCount() const {
  return SIZEOFMASS(unqliteCommands);
}

void UnqliteLexer::styleText(int start, int end) {
  if(!editor()){
    return;
  }

  char *data = new char[end - start + 1];
  editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, data);
  QString source(data);
  delete [] data;

  if(source.isEmpty()){
    return;
  }

  paintCommands(source, start);

  int index = 0;
  int begin = 0;
  while( (begin = source.indexOf(help, index, Qt::CaseInsensitive)) != -1){
    index = begin + help.length();

    startStyling(start + begin);
    setStyling(help.length(), HelpKeyword);
    startStyling(start + begin);
  }
}

void UnqliteLexer::paintCommands(const QString& source, int start) {
  for(int i = 0; i < SIZEOFMASS(unqliteCommands); ++i){
    CommandInfo cmd = unqliteCommands[i];
    QString word = common::convertFromString<QString>(cmd.name_);
    int index = 0;
    int begin = 0;
    while( (begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1){
      index = begin + word.length();

      startStyling(start + begin);
      setStyling(word.length(), Command);
      startStyling(start + begin);
    }
  }
}

}
