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

#include "core/db/leveldb/config.h"

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

#include "leveldb/options.h"  // for Options

#include "core/logger.h"

namespace fastonosql {
namespace core {
namespace leveldb {

namespace {

Config parseOptions(int argc, char** argv) {
  Config cfg;
  for (int i = 0; i < argc; i++) {
    bool lastarg = i == argc - 1;

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
        std::string buff = common::MemSPrintf(
            "Unrecognized option or bad number of args "
            "for: '%s'",
            argv[i]);
        LOG_CORE_MSG(buff, common::logging::L_WARNING, true);
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
    : LocalConfig(common::file_system::prepare_path("~/test.leveldb")),
      create_if_missing(false),
      comparator(COMP_BYTEWISE) {}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(const fastonosql::core::leveldb::Config& conf) {
  fastonosql::core::config_args_t argv = conf.Args();

  if (conf.create_if_missing) {
    argv.push_back("-c");
  }

  argv.push_back("-comp");
  argv.push_back(common::ConvertToString(conf.comparator));
  return fastonosql::core::ConvertToStringConfigArgs(argv);
}

bool ConvertFromString(const std::string& from, fastonosql::core::leveldb::Config* out) {
  if (!out) {
    return false;
  }

  int argc = 0;
  sds* argv = sdssplitargslong(from.c_str(), &argc);
  if (argv) {
    *out = fastonosql::core::leveldb::parseOptions(argc, argv);
    sdsfreesplitres(argv, argc);
    return true;
  }

  return false;
}

std::string ConvertToString(fastonosql::core::leveldb::ComparatorType comp) {
  return fastonosql::core::leveldb::g_comparator_types[comp];
}

bool ConvertFromString(const std::string& from, fastonosql::core::leveldb::ComparatorType* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(fastonosql::core::leveldb::g_comparator_types); ++i) {
    if (from == fastonosql::core::leveldb::g_comparator_types[i]) {
      *out = static_cast<fastonosql::core::leveldb::ComparatorType>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

}  // namespace common
