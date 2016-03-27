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

#include "core/leveldb/leveldb_settings.h"

#include <string>

namespace fastonosql {
namespace core {
namespace leveldb {

LeveldbConnectionSettings::LeveldbConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsLocal(connectionName, LEVELDB), info_() {
}

std::string LeveldbConnectionSettings::path() const {
  return info_.dbname;
}

std::string LeveldbConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void LeveldbConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<LeveldbConfig>(line);
}

LeveldbConfig LeveldbConnectionSettings::info() const {
  return info_;
}

void LeveldbConnectionSettings::setInfo(const LeveldbConfig& info) {
  info_ = info;
}

std::string LeveldbConnectionSettings::fullAddress() const {
  return info_.dbname;
}

LeveldbConnectionSettings* LeveldbConnectionSettings::clone() const {
  LeveldbConnectionSettings* red = new LeveldbConnectionSettings(*this);
  return red;
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
