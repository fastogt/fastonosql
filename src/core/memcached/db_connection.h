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

#include <libmemcached/memcached.h>

#include <string>

#include "core/command_handler.h"
#include "core/connection.h"

#include "core/memcached/connection_settings.h"
#include "core/memcached/config.h"
#include "core/memcached/server_info.h"

namespace fastonosql {
namespace core {
namespace memcached {

typedef memcached_st NativeConnection;

common::Error createConnection(const Config& config, NativeConnection** context);
common::Error createConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error testConnection(ConnectionSettings* settings);

struct DBConnection
  : public CommandHandler {
 public:
  typedef ConnectionAllocatorTraits<NativeConnection, Config> ConnectionAllocatorTrait;
  typedef Connection<ConnectionAllocatorTrait> connection_t;
  typedef connection_t::config_t config_t;
  DBConnection();

  common::Error connect(const config_t& config) WARN_UNUSED_RESULT;
  common::Error disconnect() WARN_UNUSED_RESULT;
  bool isConnected() const;

  std::string delimiter() const;
  std::string nsSeparator() const;
  config_t config() const;
  static const char* versionApi();

  common::Error keys(const char* args) WARN_UNUSED_RESULT;
  common::Error info(const char* args, ServerInfo::Common* statsout) WARN_UNUSED_RESULT;
  common::Error dbkcount(size_t* size) WARN_UNUSED_RESULT;

  common::Error get(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error set(const std::string& key, const std::string& value,
                    time_t expiration, uint32_t flags) WARN_UNUSED_RESULT;
  common::Error add(const std::string& key, const std::string& value,
                    time_t expiration, uint32_t flags) WARN_UNUSED_RESULT;
  common::Error replace(const std::string& key, const std::string& value,
                        time_t expiration, uint32_t flags)WARN_UNUSED_RESULT;
  common::Error append(const std::string& key, const std::string& value,
                       time_t expiration, uint32_t flags) WARN_UNUSED_RESULT;
  common::Error prepend(const std::string& key, const std::string& value,
                        time_t expiration, uint32_t flags) WARN_UNUSED_RESULT;
  common::Error incr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error decr(const std::string& key, uint64_t value) WARN_UNUSED_RESULT;
  common::Error del(const std::string& key, time_t expiration) WARN_UNUSED_RESULT;
  common::Error flush_all(time_t expiration) WARN_UNUSED_RESULT;
  common::Error version_server() const WARN_UNUSED_RESULT;
  common::Error help(int argc, char** argv) WARN_UNUSED_RESULT;

 private:
  connection_t connection_;
};

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error stats(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error add(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error replace(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error append(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error prepend(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error incr(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error decr(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error flush_all(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error version_server(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error dbkcount(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);

// TODO: cas command implementation
static const std::vector<CommandHolder> memcachedCommands = {
  CommandHolder("VERSION", "-",
              "Return the Memcached server version.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0, &version_server),
  CommandHolder("STATS", "[<args>]",
              "These command can return various stats that we will explain.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1, &stats),
  CommandHolder("FLUSH_ALL", "[<time>]",
              "Flush the server key/value pair (invalidating them) after a optional [<time>] period.\n"
              "It always return OK", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1, &flush_all),
  CommandHolder("DELETE", "<key> [<time>]",
              "Delete key/value pair in Memcached", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 1, &del),
  CommandHolder("INCR", "<key> <value>",
              "Increment value associated with key in Memcached, item must exist, increment command will not create it.\n"
              "The limit of increment is the 64 bit mark.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &incr),
  CommandHolder("DECR", "<key> <value>",
              "Decrement value associated with key in Memcached, item must exist, decrement command will not create it.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &decr),
  CommandHolder("PREPEND", "<key> <flags> <exptime> <bytes>",
              "Add value to an existing key before existing data.\n"
              "Prepend does not take <flags> or <exptime> parameters but you must provide them!",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &prepend),
  CommandHolder("APPEND", "<key> <flags> <exptime> <value>",
              "Add value to an existing key after existing data.\n"
              "Append does not take <flags> or <exptime> parameters but you must provide them!",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &append),
  CommandHolder("REPLACE", "<key> <flags> <exptime> <value>",
              "Store key/value pair in Memcached, but only if the server already hold data for this key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &replace),
  CommandHolder("ADD", "<key> <flags> <exptime> <value>",
              "Store key/value pair in Memcached, but only if the server doesn't already hold data for this key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &add),
  CommandHolder("SET", "<key> <flags> <exptime> <value>",
              "Set the string value of a key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &set),
  CommandHolder("GET", "<key>",
              "Get the value of a key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &get),

  CommandHolder("DBKCOUNT", "-",
              "Return the number of keys in the selected database",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0, &dbkcount),
  CommandHolder("HELP", "<command>",
              "Return how to use command",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1, &help)
};

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
