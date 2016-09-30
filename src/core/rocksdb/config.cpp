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

#include "core/rocksdb/config.h"

#include <stddef.h>  // for size_t
#include <string.h>  // for strcmp

#include <string>  // for string, basic_string
#include <vector>  // for vector

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>
#include <common/file_system.h>  // for prepare_path
#include <common/log_levels.h>   // for LEVEL_LOG::L_WARNING
#include <common/sprintf.h>      // for MemSPrintf

#include <common/qt/logger.h>  // for LOG_MSG

namespace fastonosql {
namespace core {
namespace rocksdb {

namespace {

Config parseOptions(int argc, char** argv) {
  Config cfg;
  for (int i = 0; i < argc; i++) {
    const bool lastarg = i == argc - 1;

    if (!strcmp(argv[i], "-d") && !lastarg) {
      cfg.delimiter = argv[++i];
    } else if (!strcmp(argv[i], "-ns") && !lastarg) {
      cfg.ns_separator = argv[++i];
    } else if (!strcmp(argv[i], "-f") && !lastarg) {
      cfg.dbname = argv[++i];
    } else if (!strcmp(argv[i], "-c")) {
      cfg.options.create_if_missing = true;
    } else {
      if (argv[i][0] == '-') {
        const std::string buff = common::MemSPrintf(
            "Unrecognized option or bad number of args "
            "for: '%s'",
            argv[i]);
        LOG_MSG(buff, common::logging::L_WARNING, true);
        break;
      } else {
        /* Likely the command name, stop here. */
        break;
      }
    }
  }
  return cfg;
}

}  // namespace

Config::Config() : LocalConfig(common::file_system::prepare_path("~/test.rocksdb")) {
  options.create_if_missing = false;
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(const fastonosql::core::rocksdb::Config& conf) {
  std::vector<std::string> argv = conf.args();

  if (conf.options.create_if_missing) {
    argv.push_back("-c");
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

template <>
fastonosql::core::rocksdb::Config ConvertFromString(const std::string& line) {
  int argc = 0;
  sds* argv = sdssplitargslong(line.c_str(), &argc);
  if (argv) {
    auto cfg = fastonosql::core::rocksdb::parseOptions(argc, argv);
    sdsfreesplitres(argv, argc);
    return cfg;
  }

  return fastonosql::core::rocksdb::Config();
}

}  // namespace common
