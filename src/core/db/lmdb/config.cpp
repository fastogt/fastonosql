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

#include "core/db/lmdb/config.h"

#include <lmdb.h>  // for mdb_txn_abort, MDB_val

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>
#include <common/file_system/types.h>  // for prepare_path
#include <common/sprintf.h>            // for MemSPrintf

#include "core/logger.h"

#define LMDB_DEFAULT_ENV_FLAGS MDB_NOSUBDIR

namespace fastonosql {
namespace core {
namespace lmdb {

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
    } else if (!strcmp(argv[i], "-m") && !lastarg) {
      unsigned int max_dbs;
      if (common::ConvertFromString(argv[++i], &max_dbs)) {
        cfg.max_dbs = max_dbs;
      }
    } else if (!strcmp(argv[i], "-e") && !lastarg) {
      int env_flags;
      if (common::ConvertFromString(argv[++i], &env_flags)) {
        cfg.env_flags = env_flags;
      }
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

Config::Config()
    : LocalConfig(common::file_system::prepare_path("~/test.lmdb")),
      env_flags(LMDB_DEFAULT_ENV_FLAGS),
      db_name(default_db_name),
      max_dbs(default_dbs_count) {}

bool Config::ReadOnlyDB() const {
  return env_flags & MDB_RDONLY;
}

bool Config::IsSingleFileDB() const {
  return env_flags & MDB_NOSUBDIR;
}

void Config::SetSingleFileDB(bool single) {
  if (single) {
    env_flags |= MDB_NOSUBDIR;
  } else {
    env_flags &= ~MDB_NOSUBDIR;
  }
}

void Config::SetReadOnlyDB(bool ro) {
  if (ro) {
    env_flags |= MDB_RDONLY;
  } else {
    env_flags &= ~MDB_RDONLY;
  }
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(const fastonosql::core::lmdb::Config& conf) {
  fastonosql::core::config_args_t argv = conf.Args();
  argv.push_back("-e");
  argv.push_back(common::ConvertToString(conf.env_flags));

  if (!conf.db_name.empty()) {
    argv.push_back("-n");
    argv.push_back(conf.db_name);
  }

  argv.push_back("-m");
  argv.push_back(common::ConvertToString(conf.max_dbs));

  return fastonosql::core::ConvertToStringConfigArgs(argv);
}

bool ConvertFromString(const std::string& from, fastonosql::core::lmdb::Config* out) {
  if (!out || from.empty()) {
    return false;
  }

  int argc = 0;
  sds* argv = sdssplitargslong(from.c_str(), &argc);
  if (argv) {
    *out = fastonosql::core::lmdb::ParseOptions(argc, argv);
    sdsfreesplitres(argv, argc);
    return true;
  }

  return false;
}

}  // namespace common
