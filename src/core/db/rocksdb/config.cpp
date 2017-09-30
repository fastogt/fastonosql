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

#include "core/db/rocksdb/config.h"

extern "C" {
#include "sds.h"
}

#include <common/file_system/types.h>  // for prepare_path
#include <common/sprintf.h>            // for MemSPrintf

#include "core/logger.h"

namespace fastonosql {
namespace core {
namespace rocksdb {

const std::vector<const char*> g_comparator_types = {"BYTEWISE", "REVERSE_BYTEWISE"};

namespace {

Config ParseOptions(int argc, char** argv) {
  Config cfg;
  for (int i = 0; i < argc; i++) {
    const bool lastarg = i == argc - 1;

    if (!strcmp(argv[i], "-d") && !lastarg) {
      cfg.delimiter = argv[++i];
    } else if (!strcmp(argv[i], "-ns") && !lastarg) {
      cfg.ns_separator = argv[++i];
    } else if (!strcmp(argv[i], "-f") && !lastarg) {
      cfg.db_path = argv[++i];
    } else if (!strcmp(argv[i], "-c")) {
      cfg.create_if_missing = true;
    } else if (!strcmp(argv[i], "-comp") && !lastarg) {
      ComparatorType lcomparator;
      if (common::ConvertFromString(argv[++i], &lcomparator)) {
        cfg.comparator = lcomparator;
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

Config::Config()
    : LocalConfig(common::file_system::prepare_path("~/test.rocksdb")),
      create_if_missing(false),
      comparator(COMP_BYTEWISE) {}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(const fastonosql::core::rocksdb::Config& conf) {
  fastonosql::core::config_args_t argv = conf.Args();

  if (conf.create_if_missing) {
    argv.push_back("-c");
  }

  argv.push_back("-comp");
  argv.push_back(common::ConvertToString(conf.comparator));
  return fastonosql::core::ConvertToStringConfigArgs(argv);
}

bool ConvertFromString(const std::string& from, fastonosql::core::rocksdb::Config* out) {
  if (!out || from.empty()) {
    return false;
  }

  int argc = 0;
  sds* argv = sdssplitargslong(from.c_str(), &argc);
  if (argv) {
    *out = fastonosql::core::rocksdb::ParseOptions(argc, argv);
    sdsfreesplitres(argv, argc);
    return true;
  }

  return false;
}

std::string ConvertToString(fastonosql::core::rocksdb::ComparatorType comp) {
  return fastonosql::core::rocksdb::g_comparator_types[comp];
}

bool ConvertFromString(const std::string& from, fastonosql::core::rocksdb::ComparatorType* out) {
  if (!out || from.empty()) {
    return false;
  }

  for (size_t i = 0; i < fastonosql::core::rocksdb::g_comparator_types.size(); ++i) {
    if (from == fastonosql::core::rocksdb::g_comparator_types[i]) {
      *out = static_cast<fastonosql::core::rocksdb::ComparatorType>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

}  // namespace common
