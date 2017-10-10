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

#include "core/config/config.h"

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>

namespace fastonosql {
namespace core {

const char BaseConfig::default_delimiter[] = "\n";

BaseConfig::BaseConfig() : delimiter(default_delimiter) {}

LocalConfig::LocalConfig(const std::string& db_path) : BaseConfig(), db_path(db_path) {}

config_args_t LocalConfig::Args() const {
  config_args_t argv;

  if (!db_path.empty()) {
    argv.push_back("-f");
    argv.push_back(db_path);
  }

  if (!delimiter.empty()) {
    argv.push_back("-d");
    argv.push_back(delimiter);
  }

  return argv;
}

RemoteConfig::RemoteConfig(const common::net::HostAndPort& host) : BaseConfig(), host(host) {}

config_args_t RemoteConfig::Args() const {
  config_args_t argv;

  if (host.IsValid()) {
    argv.push_back("-h");
    argv.push_back(host.GetHost());
    argv.push_back("-p");
    argv.push_back(common::ConvertToString(host.GetPort()));
  }

  if (!delimiter.empty()) {
    argv.push_back("-d");
    argv.push_back(delimiter);
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
