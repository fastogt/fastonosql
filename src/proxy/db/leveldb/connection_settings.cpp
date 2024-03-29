/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "proxy/db/leveldb/connection_settings.h"

namespace fastonosql {
namespace proxy {
namespace leveldb {

ConnectionSettings::ConnectionSettings(const connection_path_t& connection_path, const std::string& log_directory)
    : IConnectionSettingsLocal(connection_path, log_directory, core::LEVELDB), info_() {}

core::leveldb::Config ConnectionSettings::GetInfo() const {
  return info_;
}

void ConnectionSettings::SetInfo(const core::leveldb::Config& info) {
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
  std::string result;
  core::ConvertToStringConfigArgs(info_.ToArgs(), &result);
  return result;
}

void ConnectionSettings::SetCommandLine(const std::string& line) {
  core::config_args_t args;
  if (core::ConvertToConfigArgsString(line, &args)) {
    info_.Init(args);
  }
}

ConnectionSettings* ConnectionSettings::Clone() const {
  return new ConnectionSettings(*this);
}

}  // namespace leveldb
}  // namespace proxy
}  // namespace fastonosql
