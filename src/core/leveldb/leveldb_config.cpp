/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/leveldb/leveldb_config.h"

#include <string>
#include <vector>

#include "common/sprintf.h"
#include "common/file_system.h"

#include "fasto/qt/logger.h"

namespace fastonosql {
namespace {

void parseOptions(int argc, char **argv, leveldbConfig& cfg) {
  for (int i = 0; i < argc; i++) {
    int lastarg = i == argc-1;

    if (!strcmp(argv[i], "-d") && !lastarg) {
        cfg.mb_delim_ = argv[++i];
    } else if (!strcmp(argv[i], "-f") && !lastarg) {
        cfg.dbname_ = argv[++i];
    } else if (!strcmp(argv[i], "-c")) {
        cfg.options_.create_if_missing = true;
    } else {
      if (argv[i][0] == '-') {
        const uint16_t size_buff = 256;
        char buff[size_buff] = {0};
        common::SNPrintf(buff, sizeof(buff), "Unrecognized option or bad number of args for: '%s'",
                         argv[i]);
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

leveldbConfig::leveldbConfig()
  : LocalConfig(common::file_system::prepare_path("~/test.leveldb")) {
  options_.create_if_missing = false;
}

}  // namespace fastonosql

namespace common {

std::string convertToString(const fastonosql::leveldbConfig &conf) {
  std::vector<std::string> argv = conf.args();

  if (conf.options_.create_if_missing) {
    argv.push_back("-c");
  }

  std::string result;
  for (size_t i = 0; i < argv.size(); ++i) {
    result += argv[i];
    if (i != argv.size()-1) {
      result += " ";
    }
  }

  return result;
}

template<>
fastonosql::leveldbConfig convertFromString(const std::string& line) {
  fastonosql::leveldbConfig cfg;
  enum { kMaxArgs = 64 };
  int argc = 0;
  char *argv[kMaxArgs] = {0};

  char* p2 = strtok((char*)line.c_str(), " ");
  while (p2) {
    argv[argc++] = p2;
    p2 = strtok(0, " ");
  }

  fastonosql::parseOptions(argc, argv, cfg);
  return cfg;
}

}  // namespace common
