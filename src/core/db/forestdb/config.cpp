/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "core/db/forestdb/config.h"

extern "C" {
#include "sds_fasto.h"
}

#include <common/file_system/types.h>  // for prepare_path
#include <common/sprintf.h>            // for MemSPrintf

#include "core/logger.h"

namespace fastonosql {
namespace core {
namespace forestdb {

namespace {

Config ParseOptions(int argc, char** argv) {
  Config cfg;
  for (int i = 0; i < argc; i++) {
    const bool lastarg = i == argc - 1;

    if (!strcmp(argv[i], "-d") && !lastarg) {
      cfg.delimiter = argv[++i];
    } else if (!strcmp(argv[i], "-f") && !lastarg) {
      cfg.db_path = argv[++i];
    } else if (!strcmp(argv[i], "-n") && !lastarg) {
      cfg.db_name = argv[++i];
    } else {
      if (argv[i][0] == '-') {
        const std::string buff = common::MemSPrintf(
            "Unrecognized option or bad number of args "
            "for: '%s'",
            argv[i]);
        LOG_CORE_MSG(buff, common::logging::LOG_LEVEL_WARNING, true);
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

const std::string Config::default_db_name = "default";
Config::Config() : LocalConfig(common::file_system::prepare_path("~/test.forestdb")), db_name(default_db_name) {}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(const fastonosql::core::forestdb::Config& conf) {
  fastonosql::core::config_args_t argv = conf.Args();

  if (!conf.db_name.empty()) {
    argv.push_back("-n");
    argv.push_back(conf.db_name);
  }

  return fastonosql::core::ConvertToStringConfigArgs(argv);
}

bool ConvertFromString(const std::string& from, fastonosql::core::forestdb::Config* out) {
  if (!out || from.empty()) {
    return false;
  }

  int argc = 0;
  sds* argv = sdssplitargslong(from.c_str(), &argc);
  if (argv) {
    *out = fastonosql::core::forestdb::ParseOptions(argc, argv);
    sdsfreesplitres(argv, argc);
    return true;
  }

  return false;
}

}  // namespace common
