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

#include "core/ssdb/ssdb_config.h"

#include <string>
#include <vector>

#include "common/sprintf.h"

#include "fasto/qt/logger.h"

#define DEFAULT_SSDB_SERVER_PORT 8888

namespace fastonosql {
namespace core {
namespace ssdb {
namespace {

void parseOptions(int argc, char** argv, Config& cfg) {
  for (int i = 0; i < argc; i++) {
    int lastarg = i == argc - 1;

    if (!strcmp(argv[i], "-h") && !lastarg) {
      cfg.host.host = argv[++i];
    } else if (!strcmp(argv[i], "-p") && !lastarg) {
      cfg.host.port = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-u") && !lastarg) {
      cfg.user = argv[++i];
    } else if (!strcmp(argv[i], "-a") && !lastarg) {
      cfg.password = argv[++i];
    } else if (!strcmp(argv[i], "-d") && !lastarg) {
      cfg.delimiter = argv[++i];
    } else if (!strcmp(argv[i], "-ns") && !lastarg) {
      cfg.ns_separator = argv[++i];
    } else {
      if (argv[i][0] == '-') {
        std::string buff = common::MemSPrintf("Unrecognized option or bad number of args for: '%s'", argv[i]);
        LOG_MSG(buff, common::logging::L_WARNING, true);
        break;
      } else {
          /* Likely the command name, stop here. */
          break;
      }
    }
  }
}

}  // namespace

Config::Config()
  : RemoteConfig(common::net::hostAndPort::createLocalHost(DEFAULT_SSDB_SERVER_PORT)), user(), password() {
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string convertToString(const fastonosql::core::ssdb::Config& conf) {
  std::vector<std::string> argv = conf.args();

  if (!conf.user.empty()) {
    argv.push_back("-u");
    argv.push_back(conf.user);
  }

  if (!conf.password.empty()) {
    argv.push_back("-a");
    argv.push_back(conf.password);
  }

  std::string result;
  for (size_t i = 0; i < argv.size(); ++i) {
    result += argv[i];
    if (i != argv.size() - 1) {
      result += " ";
    }
  }

  return result;
}

template<>
fastonosql::core::ssdb::Config convertFromString(const std::string& line) {
  fastonosql::core::ssdb::Config cfg;
  enum { kMaxArgs = 64 };
  int argc = 0;
  char* argv[kMaxArgs] = {0};

  char* p2 = strtok((char*)line.c_str(), " ");
  while (p2) {
    argv[argc++] = p2;
    p2 = strtok(0, " ");
  }

  fastonosql::core::ssdb::parseOptions(argc, argv, cfg);
  return cfg;
}

}  // namespace common
