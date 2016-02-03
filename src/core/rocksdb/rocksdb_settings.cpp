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

#include "core/rocksdb/rocksdb_settings.h"

#include <string>

namespace fastonosql {

RocksdbConnectionSettings::RocksdbConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsBase(connectionName, ROCKSDB), info_() {
}

std::string RocksdbConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void RocksdbConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<rocksdbConfig>(line);
}

rocksdbConfig RocksdbConnectionSettings::info() const {
  return info_;
}

void RocksdbConnectionSettings::setInfo(const rocksdbConfig &info) {
  info_ = info;
}

std::string RocksdbConnectionSettings::fullAddress() const {
  return info_.dbname;
}

IConnectionSettings* RocksdbConnectionSettings::clone() const {
  RocksdbConnectionSettings *red = new RocksdbConnectionSettings(*this);
  return red;
}

std::string RocksdbConnectionSettings::toCommandLine() const {
  std::string result = common::convertToString(info_);
  return result;
}

void RocksdbConnectionSettings::initFromCommandLine(const std::string& val) {
  info_ = common::convertFromString<rocksdbConfig>(val);
}

}  // namespace fastonosql
