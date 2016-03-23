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

#include "core/ssdb/ssdb_settings.h"

#include <string>

#include "common/utils.h"

namespace fastonosql {
namespace ssdb {

SsdbConnectionSettings::SsdbConnectionSettings(const std::string& connectionName)
  : IConnectionSettingsRemote(connectionName, SSDB), info_() {
}

std::string SsdbConnectionSettings::commandLine() const {
  return common::convertToString(info_);
}

void SsdbConnectionSettings::setCommandLine(const std::string& line) {
  info_ = common::convertFromString<SsdbConfig>(line);
}

void SsdbConnectionSettings::setHost(const common::net::hostAndPort& host) {
  info_.host = host;
}

common::net::hostAndPort SsdbConnectionSettings::host() const {
  return info_.host;
}

SsdbConfig SsdbConnectionSettings::info() const {
  return info_;
}

void SsdbConnectionSettings::setInfo(const SsdbConfig& info) {
  info_ = info;
}

SsdbConnectionSettings* SsdbConnectionSettings::clone() const {
  SsdbConnectionSettings* red = new SsdbConnectionSettings(*this);
  return red;
}

}  // namespace ssdb
}  // namespace fastonosql
