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

#include "core/memcached/memcached_settings.h"

#include <string>

#include "common/utils.h"

namespace fastonosql {
namespace memcached {

MemcachedConnectionSettings::MemcachedConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsRemote(connectionName, MEMCACHED), info_() {
}

std::string MemcachedConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void MemcachedConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<MemcachedConfig>(line);
}

void MemcachedConnectionSettings::setHost(const common::net::hostAndPort& host) {
  info_.host = host;
}

common::net::hostAndPort MemcachedConnectionSettings::host() const {
  return info_.host;
}

MemcachedConfig MemcachedConnectionSettings::info() const {
  return info_;
}

void MemcachedConnectionSettings::setInfo(const MemcachedConfig& info) {
  info_ = info;
}

MemcachedConnectionSettings* MemcachedConnectionSettings::clone() const {
  MemcachedConnectionSettings* red = new MemcachedConnectionSettings(*this);
  return red;
}

}  // namespace memcached
}  // namespace fastonosql
