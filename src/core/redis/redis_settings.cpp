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

#include "core/redis/redis_settings.h"

#include <string>

#include "common/utils.h"

namespace fastonosql {
namespace redis {

RedisConnectionSettings::RedisConnectionSettings(const std::string &connectionName)
  : IConnectionSettingsRemote(connectionName, REDIS), info_() {
}

void RedisConnectionSettings::setHost(const common::net::hostAndPort& host) {
  info_.host = host;
}

common::net::hostAndPort RedisConnectionSettings::host() const {
  return info_.host;
}

void RedisConnectionSettings::setCommandLine(const std::string &line) {
  info_ = common::convertFromString<RedisConfig>(line);
}

std::string RedisConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

RedisConfig RedisConnectionSettings::info() const {
  return info_;
}

void RedisConnectionSettings::setInfo(const RedisConfig &info) {
  info_ =  info;
}

IConnectionSettings *RedisConnectionSettings::clone() const {
  RedisConnectionSettings *red = new RedisConnectionSettings(*this);
  return red;
}

}  // namespace redis
}  // namespace fastonosql
