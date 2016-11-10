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

#include "core/config/config.h"

#include <string>
#include <vector>

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>

namespace fastonosql {
namespace core {

LocalConfig::LocalConfig(const std::string& dbname) : BaseConfig<LOCAL>(), dbname(dbname) {}

config_args_t LocalConfig::Args() const {
  config_args_t argv;

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

RemoteConfig::RemoteConfig(const common::net::HostAndPort& host)
    : BaseConfig<REMOTE>(), host(host) {}

config_args_t RemoteConfig::Args() const {
  config_args_t argv;

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

std::string ConvertToStringConfigArgs(const config_args_t& args) {
  std::string result;
  for (size_t i = 0; i < args.size(); ++i) {
    std::string curr = args[i];
    if (is_need_escape(curr.c_str(), curr.length())) {
      result += "'" + args[i] + "'";
    } else {
      result += args[i];
    }
    if (i != args.size() - 1) {
      result += " ";
    }
  }

  return result;
}

}  // namespace core
}  // namespace fastonosql
