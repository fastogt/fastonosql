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

#pragma once

extern "C" {
  #include <lmdb.h>
}

#include <string>

#include "core/db_connection.h"
#include "core/command_handler.h"

#include "core/lmdb/lmdb_settings.h"
#include "core/lmdb/config.h"
#include "core/lmdb/lmdb_infos.h"

namespace fastonosql {
namespace core {
namespace lmdb {

struct lmdb {
  MDB_env* env;
  MDB_dbi dbir;
};

typedef lmdb LMDBConnection;
typedef DBAllocatorTraits<LMDBConnection, Config> LMDBAllocTrait;

common::Error createConnection(const Config& config, LMDBConnection** context);
common::Error createConnection(LmdbConnectionSettings* settings, LMDBConnection** context);
common::Error testConnection(LmdbConnectionSettings* settings);

struct LmdbRaw
  : public CommandHandler {
  typedef DBConnection<LMDBAllocTrait> connection_t;
  typedef connection_t::config_t config_t;
  LmdbRaw();

  common::Error connect(const config_t& config) WARN_UNUSED_RESULT;
  common::Error disconnect() WARN_UNUSED_RESULT;
  bool isConnected() const;

  std::string delimiter() const;
  std::string nsSeparator() const;
  config_t config() const;

  static const char* versionApi();

  MDB_dbi curDb() const;

  common::Error info(const char* args, LmdbServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error set(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error get(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error del(const std::string& key) WARN_UNUSED_RESULT;
  common::Error keys(const std::string& key_start, const std::string& key_end, uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;

  // extended api
  common::Error dbsize(size_t* size) WARN_UNUSED_RESULT;
  common::Error help(int argc, char** argv) WARN_UNUSED_RESULT;
  common::Error flushdb() WARN_UNUSED_RESULT;

 private:
  connection_t connection_;
};

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out);

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out);

static const std::vector<CommandHolder> lmdbCommands = {
  CommandHolder("SET", "<key> <value>",
              "Set the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &set),
  CommandHolder("GET", "<key>",
              "Get the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &get),
  CommandHolder("DEL", "<key>",
              "Delete key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &del),
  CommandHolder("KEYS", "<key_start> <key_end> <limit>",
              "Find all keys matching the given limits.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0, &keys),
  CommandHolder("INFO", "<args>",
              "These command return database information.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &info),

  // extended commands
  CommandHolder("DBSIZE", "-",
              "Return the number of keys in the selected database",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0, &dbsize),
  CommandHolder("HELP", "<command>",
              "Return how to use command",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1, &help),
  CommandHolder("FLUSHDB", "-",
              "Remove all keys from the current database",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1, &flushdb)
};

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
