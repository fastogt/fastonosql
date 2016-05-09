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

#include "core/unqlite/unqlite_settings.h"

#include <string>

namespace fastonosql {
namespace core {
namespace unqlite {

UnqliteConnectionSettings::UnqliteConnectionSettings(const connection_path_t& connectionName)
  : IConnectionSettingsLocal(connectionName, UNQLITE), info_() {
}

std::string UnqliteConnectionSettings::dbpath() const {
  return info_.dbname;
}

std::string UnqliteConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void UnqliteConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<Config>(line);
}

Config UnqliteConnectionSettings::info() const {
  return info_;
}

void UnqliteConnectionSettings::setInfo(const Config &info) {
  info_ = info;
}

std::string UnqliteConnectionSettings::fullAddress() const {
  return info_.dbname;
}

UnqliteConnectionSettings* UnqliteConnectionSettings::clone() const {
  UnqliteConnectionSettings* red = new UnqliteConnectionSettings(*this);
  return red;
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
