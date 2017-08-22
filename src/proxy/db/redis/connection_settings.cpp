/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/db/redis/connection_settings.h"

#include <string>  // for string

#include <common/convert2string.h>
#include <common/net/types.h>  // for HostAndPort

#include "proxy/connection_settings/iconnection_settings_ssh.h"

#include "core/db/redis/config.h"  // for Config

namespace fastonosql {
namespace proxy {
namespace redis {

ConnectionSettings::ConnectionSettings(const connection_path_t& connectionName)
    : IConnectionSettingsRemoteSSH(connectionName, core::REDIS), info_() {}

std::string ConnectionSettings::GetDelimiter() const {
  return info_.delimiter;
}

void ConnectionSettings::SetDelimiter(const std::string& delimiter) {
  info_.delimiter = delimiter;
}

std::string ConnectionSettings::GetNsSeparator() const {
  return info_.ns_separator;
}

void ConnectionSettings::SetNsSeparator(const std::string& ns) {
  info_.ns_separator = ns;
}

common::net::HostAndPort ConnectionSettings::Host() const {
  return info_.host;
}

void ConnectionSettings::SetHost(const common::net::HostAndPort& host) {
  info_.host = host;
}

void ConnectionSettings::SetCommandLine(const std::string& line) {
  core::redis::Config lifo;
  if (common::ConvertFromString(line, &lifo)) {
    info_ = lifo;
  }
}

std::string ConnectionSettings::CommandLine() const {
  return common::ConvertToString(info_);
}

core::redis::Config ConnectionSettings::GetInfo() const {
  return info_;
}

void ConnectionSettings::SetInfo(const core::redis::Config& info) {
  info_ = info;
}

ConnectionSettings* ConnectionSettings::Clone() const {
  ConnectionSettings* red = new ConnectionSettings(*this);
  return red;
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
