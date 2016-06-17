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

#include "core/connection_confg.h"

#include <string>
#include <vector>

#include "common/convert2string.h"

namespace fastonosql {
namespace core {

LocalConfig::LocalConfig(const std::string& dbname)
  : BaseConfig<LOCAL>(), dbname(dbname) {
}

std::vector<std::string> LocalConfig::args() const {
  std::vector<std::string> argv;

  if (!dbname.empty()) {
    argv.push_back("-f");
    argv.push_back(dbname);
  }

  if (!delimiter.empty()) {
    argv.push_back("-d");
    argv.push_back(delimiter);
  }

  if (!ns_separator.empty()) {
    argv.push_back("-ns");
    argv.push_back(ns_separator);
  }

  return argv;
}

RemoteConfig::RemoteConfig(const common::net::hostAndPort& host)
  : BaseConfig<REMOTE>(), host(host) {
}

std::vector<std::string> RemoteConfig::args() const {
  std::vector<std::string> argv;

  if (host.isValid()) {
    argv.push_back("-h");
    argv.push_back(host.host);
    argv.push_back("-p");
    argv.push_back(common::ConvertToString(host.port));
  }

  if (!delimiter.empty()) {
    argv.push_back("-d");
    argv.push_back(delimiter);
  }

  if (!ns_separator.empty()) {
    argv.push_back("-ns");
    argv.push_back(ns_separator);
  }

  return argv;
}

}  // namespace core
}  // namespace fastonosql
