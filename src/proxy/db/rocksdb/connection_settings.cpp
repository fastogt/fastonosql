/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "proxy/db/rocksdb/connection_settings.h"

namespace fastonosql {
namespace proxy {
namespace rocksdb {

ConnectionSettings::ConnectionSettings(const connection_path_t& connection_name, const std::string& log_directory)
    : IConnectionSettingsLocal(connection_name, log_directory, core::ROCKSDB), info_() {}

core::rocksdb::Config ConnectionSettings::GetInfo() const {
  return info_;
}

void ConnectionSettings::SetInfo(const core::rocksdb::Config& info) {
  info_ = info;
}

std::string ConnectionSettings::GetDelimiter() const {
  return info_.delimiter;
}

void ConnectionSettings::SetDelimiter(const std::string& delimiter) {
  info_.delimiter = delimiter;
}

std::string ConnectionSettings::GetDBPath() const {
  return info_.db_path;
}

void ConnectionSettings::SetDBPath(const std::string& db_path) {
  info_.db_path = db_path;
}

std::string ConnectionSettings::GetCommandLine() const {
  return common::ConvertToString(info_);
}

void ConnectionSettings::SetCommandLine(const std::string& line) {
  core::rocksdb::Config linfo;
  if (common::ConvertFromString(line, &linfo)) {
    info_ = linfo;
  }
}

ConnectionSettings* ConnectionSettings::Clone() const {
  ConnectionSettings* red = new ConnectionSettings(*this);
  return red;
}

}  // namespace rocksdb
}  // namespace proxy
}  // namespace fastonosql
