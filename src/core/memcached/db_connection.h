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

#include <stdint.h>  // for uint32_t, uint64_t
#include <time.h>    // for time_t
#include <string>    // for string

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT

#include "core/db_connection/cdb_connection.h"  // for CDBConnection
#include "core/command/command_info.h"          // for UNDEFINED_EXAMPLE_STR, UNDEF...
#include "core/connection_types.h"              // for connectionTypes::MEMCACHED
#include "core/db_key.h"                        // for NDbKValue, NKey, NKeys
#include "core/memcached/connection_settings.h"
#include "core/memcached/server_info.h"

namespace fastonosql {
class FastoObject;
}
namespace fastonosql {
namespace core {
class CDBConnectionClient;
}
}
namespace fastonosql {
namespace core {
class CommandHandler;
}
}
namespace fastonosql {
namespace core {
class IDataBaseInfo;
}
}
namespace fastonosql {
namespace core {
namespace memcached {
class ConnectionSettings;
}
}
}

struct memcached_st;  // lines 37-37

namespace fastonosql {
namespace core {
namespace memcached {

typedef memcached_st NativeConnection;

common::Error CreateConnection(const Config& config, NativeConnection** context);
common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error TestConnection(ConnectionSettings* settings);

class DBConnection : public core::CDBConnection<NativeConnection, Config, MEMCACHED> {
 public:
  typedef core::CDBConnection<NativeConnection, Config, MEMCACHED> base_class;
  DBConnection(CDBConnectionClient* client);

  static const char* VersionApi();

  common::Error Keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error DBkcount(size_t* size) WARN_UNUSED_RESULT;

  common::Error AddIfNotExist(const std::string& key,
                              const std::string& value,
                              time_t expiration,
                              uint32_t flags) WARN_UNUSED_RESULT;
  common::Error Replace(const std::string& key,
                        const std::string& value,
                        time_t expiration,
                        uint32_t flags) WARN_UNUSED_RESULT;
  common::Error Append(const std::string& key,
                       const std::string& value,
                       time_t expiration,
                       uint32_t flags) WARN_UNUSED_RESULT;
  common::Error Prepend(const std::string& key,
                        const std::string& value,
                        time_t expiration,
                        uint32_t flags) WARN_UNUSED_RESULT;
  common::Error Incr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error Decr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error Flushdb(time_t expiration) WARN_UNUSED_RESULT;
  common::Error VersionServer() const WARN_UNUSED_RESULT;
  common::Error Help(int argc, const char** argv) WARN_UNUSED_RESULT;

 private:
  common::Error DelInner(const std::string& key, time_t expiration) WARN_UNUSED_RESULT;
  common::Error GetInner(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error SetInner(const std::string& key,
                         const std::string& value,
                         time_t expiration,
                         uint32_t flags) WARN_UNUSED_RESULT;
  common::Error ExpireInner(const std::string& key, time_t expiration) WARN_UNUSED_RESULT;

  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) override;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) override;
};

common::Error select(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error rename(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set_ttl(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

common::Error keys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error stats(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error add(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error replace(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error append(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error prepend(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error incr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error decr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error version_server(CommandHandler* handler,
                             int argc,
                             const char** argv,
                             FastoObject* out);
common::Error dbkcount(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error expire(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

// TODO: cas command implementation
static const std::vector<CommandHolder> memcachedCommands = {
    CommandHolder("VERSION",
                  "-",
                  "Return the Memcached server version.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &version_server),
    CommandHolder("KEYS",
                  "<key_start> <key_end> <limit>",
                  "Find all keys matching the given limits.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &keys),
    CommandHolder("STATS",
                  "[args]",
                  "These command can return various "
                  "stats that we will explain.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &stats),
    CommandHolder("FLUSHDB",
                  "[time]",
                  "Flush the server key/value pair "
                  "(invalidating them) after a "
                  "optional [<time>] period.\n"
                  "It always return OK",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &flushdb),
    CommandHolder("DEL",
                  "<key> [key ...]",
                  "Delete key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &del),
    CommandHolder("INCR",
                  "<key> <value>",
                  "Increment value associated with key in "
                  "Memcached, item must "
                  "exist, increment "
                  "command will not create it.\n"
                  "The limit of increment is the 64 bit mark.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &incr),
    CommandHolder("DECR",
                  "<key> <value>",
                  "Decrement value associated with key "
                  "in Memcached, item must "
                  "exist, decrement "
                  "command will not create it.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &decr),
    CommandHolder("PREPEND",
                  "<key> <flags> <exptime> <bytes>",
                  "Add value to an existing key before "
                  "existing data.\n"
                  "Prepend does not take <flags> or "
                  "<exptime> parameters but "
                  "you must provide them!",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &prepend),
    CommandHolder("APPEND",
                  "<key> <flags> <exptime> <value>",
                  "Add value to an existing key after "
                  "existing data.\n"
                  "Append does not take <flags> or "
                  "<exptime> parameters but "
                  "you must provide them!",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &append),
    CommandHolder("REPLACE",
                  "<key> <flags> <exptime> <value>",
                  "Store key/value pair in Memcached, "
                  "but only if the server "
                  "already hold data for this key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &replace),
    CommandHolder("ADD",
                  "<key> <flags> <exptime> <value>",
                  "Store key/value pair in Memcached, "
                  "but only if the server "
                  "doesn't already hold "
                  "data for this key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &add),
    CommandHolder("SET",
                  "<key> <flags> <exptime> <value>",
                  "Set the string value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
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
    CommandHolder("RENAME",
                  "<key> <newkey>",
                  "Rename a key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &rename),
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
                  "[command]",
                  "Return how to use command",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &help),
    CommandHolder("EXPIRE",
                  "<key> <exptime>",
                  "Set a key's time to live in seconds",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &expire),
};
}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
