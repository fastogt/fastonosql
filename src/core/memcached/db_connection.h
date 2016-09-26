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
#include <stdint.h>  // for uint32_t, uint64_t
#include <time.h>    // for time_t

#include <string>  // for string
#include <vector>  // for vector

#include "common/error.h"   // for Error
#include "common/macros.h"  // for WARN_UNUSED_RESULT

#include "core/cdb_connection.h"

#include "core/memcached/config.h"
#include "core/memcached/connection_settings.h"
#include "core/memcached/server_info.h"

struct memcached_st;

namespace fastonosql {
namespace core {
namespace memcached {

typedef memcached_st NativeConnection;

common::Error createConnection(const Config& config, NativeConnection** context);
common::Error createConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error testConnection(ConnectionSettings* settings);

class DBConnection : public core::CDBConnection<NativeConnection, Config, MEMCACHED> {
 public:
  typedef core::CDBConnection<NativeConnection, Config, MEMCACHED> base_class;
  DBConnection(CDBConnectionClient* client);

  static const char* versionApi();

  common::Error keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error dbkcount(size_t* size) WARN_UNUSED_RESULT;

  common::Error addIfNotExist(const std::string& key,
                              const std::string& value,
                              time_t expiration,
                              uint32_t flags) WARN_UNUSED_RESULT;
  common::Error replace(const std::string& key,
                        const std::string& value,
                        time_t expiration,
                        uint32_t flags) WARN_UNUSED_RESULT;
  common::Error append(const std::string& key,
                       const std::string& value,
                       time_t expiration,
                       uint32_t flags) WARN_UNUSED_RESULT;
  common::Error prepend(const std::string& key,
                        const std::string& value,
                        time_t expiration,
                        uint32_t flags) WARN_UNUSED_RESULT;
  common::Error incr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error decr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error flush_all(time_t expiration) WARN_UNUSED_RESULT;
  common::Error version_server() const WARN_UNUSED_RESULT;
  common::Error help(int argc, const char** argv) WARN_UNUSED_RESULT;

 private:
  common::Error delInner(const std::string& key, time_t expiration) WARN_UNUSED_RESULT;
  common::Error getInner(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error setInner(const std::string& key,
                         const std::string& value,
                         time_t expiration,
                         uint32_t flags) WARN_UNUSED_RESULT;
  common::Error expireInner(const std::string& key, time_t expiration) WARN_UNUSED_RESULT;

  virtual common::Error selectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error delImpl(const keys_t& keys, keys_t* deleted_keys) override;
  virtual common::Error getImpl(const key_t& key, key_and_value_t* loaded_key) override;
  virtual common::Error setImpl(const key_and_value_t& key, key_and_value_t* added_key) override;
  virtual common::Error setTTLImpl(const key_t& key, ttl_t ttl) override;
};

common::Error select(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set_ttl(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

common::Error keys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error stats(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error add(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error replace(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error append(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error prepend(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error incr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error decr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error flush_all(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
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
                  "[<args>]",
                  "These command can return various "
                  "stats that we will explain.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &stats),
    CommandHolder("FLUSH_ALL",
                  "[<time>]",
                  "Flush the server key/value pair "
                  "(invalidating them) after a "
                  "optional [<time>] period.\n"
                  "It always return OK",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &flush_all),
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
