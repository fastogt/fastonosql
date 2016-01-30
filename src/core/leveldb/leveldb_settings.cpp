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

#include "core/leveldb/leveldb_settings.h"

namespace fastonosql {

LeveldbConnectionSettings::LeveldbConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsBase(connectionName, LEVELDB), info_() {
}

std::string LeveldbConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void LeveldbConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<leveldbConfig>(line);
}

leveldbConfig LeveldbConnectionSettings::info() const {
  return info_;
}

void LeveldbConnectionSettings::setInfo(const leveldbConfig &info) {
  info_ = info;
}

std::string LeveldbConnectionSettings::fullAddress() const {
  return info_.dbname_;
}

IConnectionSettings* LeveldbConnectionSettings::clone() const {
  LeveldbConnectionSettings *red = new LeveldbConnectionSettings(*this);
  return red;
}

std::string LeveldbConnectionSettings::toCommandLine() const {
  std::string result = common::convertToString(info_);
  return result;
}

void LeveldbConnectionSettings::initFromCommandLine(const std::string& val) {
  info_ = common::convertFromString<leveldbConfig>(val);
}

}
