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

#include <common/error.h>

#include "core/command_info.h"
#include "core/internal/command_handler.h"

namespace fastonosql {
namespace core {
namespace memcached {

common::Error select(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out);
common::Error set(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error get(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error del(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error rename(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out);
common::Error set_ttl(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out);

common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error stats(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out);
common::Error add(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error replace(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out);
common::Error append(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out);
common::Error prepend(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out);
common::Error incr(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error decr(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out);
common::Error version_server(internal::CommandHandler* handler,
                             int argc,
                             const char** argv,
                             FastoObject* out);
common::Error dbkcount(internal::CommandHandler* handler,
                       int argc,
                       const char** argv,
                       FastoObject* out);
common::Error help(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error expire(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out);
common::Error quit(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);

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
    CommandHolder("QUIT",
                  "-",
                  "Close the connection",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &quit),
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
