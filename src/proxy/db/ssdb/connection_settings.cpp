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

#include "proxy/db/ssdb/connection_settings.h"

namespace fastonosql {
namespace proxy {
namespace ssdb {

ConnectionSettings::ConnectionSettings(const connection_path_t& connection_path, const std::string& log_directory)
    : IConnectionSettingsRemote(connection_path, log_directory, core::SSDB), info_() {}

std::string ConnectionSettings::GetDelimiter() const {
  return info_.delimiter;
}

void ConnectionSettings::SetDelimiter(const std::string& delimiter) {
  info_.delimiter = delimiter;
}

common::net::HostAndPort ConnectionSettings::GetHost() const {
  return info_.host;
}

void ConnectionSettings::SetHost(const common::net::HostAndPort& host) {
  info_.host = host;
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

core::ssdb::Config ConnectionSettings::GetInfo() const {
  return info_;
}

void ConnectionSettings::SetInfo(const core::ssdb::Config& info) {
  info_ = info;
}

ConnectionSettings* ConnectionSettings::Clone() const {
  return new ConnectionSettings(*this);
}

}  // namespace ssdb
}  // namespace proxy
}  // namespace fastonosql
