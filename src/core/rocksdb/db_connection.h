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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint64_t

#include <string>  // for string
#include <vector>  // for vector

#include "common/error.h"   // for Error
#include "common/macros.h"  // for WARN_UNUSED_RESULT

#include "core/command_holder.h"  // for CommandHolder, etc
#include "core/command_info.h"    // for UNDEFINED_EXAMPLE_STR, etc
#include "core/cdb_connection.h"

#include "core/rocksdb/config.h"
#include "core/rocksdb/connection_settings.h"
#include "core/rocksdb/server_info.h"

namespace rocksdb {
class DB;
}

namespace fastonosql {
namespace core {
namespace rocksdb {

typedef ::rocksdb::DB NativeConnection;

common::Error createConnection(const Config& config, NativeConnection** context);
common::Error createConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error testConnection(ConnectionSettings* settings);

class DBConnection : public core::CDBConnection<NativeConnection, Config, ROCKSDB> {
 public:
  typedef core::CDBConnection<NativeConnection, Config, ROCKSDB> base_class;
  DBConnection(CDBConnectionClient* client);

  static const char* versionApi();

  std::string currentDbName() const;

  common::Error info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error set(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error get(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error mget(const std::vector<::rocksdb::Slice>& keys, std::vector<std::string>* ret);
  common::Error merge(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error del(const std::string& key) WARN_UNUSED_RESULT;
  common::Error keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;

  // extended api
  common::Error dbkcount(size_t* size) WARN_UNUSED_RESULT;
  common::Error help(int argc, char** argv) WARN_UNUSED_RESULT;
  common::Error flushdb() WARN_UNUSED_RESULT;

 private:
  common::Error selectImpl(const std::string& name, IDataBaseInfo** info) WARN_UNUSED_RESULT;
};

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error mget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error merge(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out);

common::Error dbkcount(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out);

static const std::vector<CommandHolder> rocksdbCommands = {
    CommandHolder("SET",
                  "<key> <value>",
                  "Set the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &set),
    CommandHolder("GET",
                  "<key>",
                  "Get the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &get),
    CommandHolder("MGET",
                  "<keys>",
                  "Get the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &mget),
    CommandHolder("MERGE",
                  "<key> <value>",
                  "Merge the database entry for \"key\" "
                  "with \"value\"",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &merge),
    CommandHolder("DEL",
                  "<key>",
                  "Delete key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &del),
    CommandHolder("KEYS",
                  "<key_start> <key_end> <limit>",
                  "Find all keys matching the given limits.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &keys),
    CommandHolder("INFO",
                  "<args>",
                  "These command return database information.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &info),

    // extended commands
    CommandHolder("DBKCOUNT",
                  "-",
                  "Return the number of keys in the "
                  "selected database",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &dbkcount),
    CommandHolder("HELP",
                  "<command>",
                  "Return how to use command",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &help),
    CommandHolder("FLUSHDB",
                  "-",
                  "Remove all keys from the current database",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &flushdb)};

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
