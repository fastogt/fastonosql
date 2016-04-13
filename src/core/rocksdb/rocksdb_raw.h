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

#include <string>

#include <rocksdb/db.h>

#include "core/rocksdb/rocksdb_settings.h"
#include "core/rocksdb/rocksdb_config.h"
#include "core/rocksdb/rocksdb_infos.h"

namespace fastonosql {
namespace core {
namespace rocksdb {

typedef ::rocksdb::DB RocksDBConnection;
typedef DBAllocatorTraits<RocksDBConnection, RocksdbConfig> RocksDBAllocTrait;

common::Error createConnection(const RocksdbConfig& config, RocksDBConnection** context);
common::Error createConnection(RocksdbConnectionSettings* settings, RocksDBConnection** context);
common::Error testConnection(RocksdbConnectionSettings* settings);

class RocksdbRaw
  : public CommandHandler {
 public:
  typedef DBConnection<RocksDBAllocTrait> connection_t;
  typedef connection_t::config_t config_t;
  RocksdbRaw();

  common::Error connect(const config_t& config) WARN_UNUSED_RESULT;
  common::Error disconnect() WARN_UNUSED_RESULT;
  bool isConnected() const;

  std::string delimiter() const;
  config_t config() const;
  static const char* versionApi();

  std::string currentDbName() const;

  common::Error info(const char* args, RocksdbServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error set(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error get(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error mget(const std::vector< ::rocksdb::Slice>& keys, std::vector<std::string>* ret);
  common::Error merge(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error del(const std::string& key) WARN_UNUSED_RESULT;
  common::Error keys(const std::string& key_start, const std::string& key_end,
                     uint64_t limit, std::vector<std::string>* ret) WARN_UNUSED_RESULT;

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
common::Error mget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error merge(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out);

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out);

static const std::vector<CommandHolder> rocksdbCommands = {
  CommandHolder("SET", "<key> <value>",
              "Set the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &set),
  CommandHolder("GET", "<key>",
              "Get the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &get),
  CommandHolder("MGET", "<keys>",
              "Get the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &mget),
  CommandHolder("MERGE", "<key> <value>",
              "Merge the database entry for \"key\" with \"value\"",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &merge),
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

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
