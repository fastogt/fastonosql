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

#include "core/lmdb/lmdb_settings.h"

#include <string>

namespace fastonosql {

LmdbConnectionSettings::LmdbConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsBase(connectionName, LMDB), info_() {
}

std::string LmdbConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void LmdbConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<lmdbConfig>(line);
}

lmdbConfig LmdbConnectionSettings::info() const {
  return info_;
}

void LmdbConnectionSettings::setInfo(const lmdbConfig &info) {
  info_ = info;
}

std::string LmdbConnectionSettings::fullAddress() const {
  return info_.dbname;
}

IConnectionSettings* LmdbConnectionSettings::clone() const {
  LmdbConnectionSettings *red = new LmdbConnectionSettings(*this);
  return red;
}

std::string LmdbConnectionSettings::toCommandLine() const {
  std::string result = common::convertToString(info_);
  return result;
}

void LmdbConnectionSettings::initFromCommandLine(const std::string& val) {
  info_ = common::convertFromString<lmdbConfig>(val);
}

}  // namespace fastonosql
