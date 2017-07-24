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

#include "core/db/memcached/db_connection.h"
#include "core/internal/commands_api.h"

namespace fastonosql {
namespace core {
namespace memcached {

struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler,
                            int argc,
                            const char** argv,
                            FastoObject* out);
  static common::Error Version(internal::CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out);

  static common::Error Add(internal::CommandHandler* handler,
                           int argc,
                           const char** argv,
                           FastoObject* out);
  static common::Error Replace(internal::CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out);
  static common::Error Append(internal::CommandHandler* handler,
                              int argc,
                              const char** argv,
                              FastoObject* out);
  static common::Error Prepend(internal::CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out);
  static common::Error Incr(internal::CommandHandler* handler,
                            int argc,
                            const char** argv,
                            FastoObject* out);
  static common::Error Decr(internal::CommandHandler* handler,
                            int argc,
                            const char** argv,
                            FastoObject* out);
};

// TODO: cas command implementation
static const internal::ConstantCommandsArray memcachedCommands = {

    CommandHolder("HELP",
                  "[command]",
                  "Return how to use command",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Help),
    CommandHolder("INFO",
                  "[section]",
                  "These command return database information.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Info),
    CommandHolder("VERSION",
                  "-",
                  "Return the Memcached server version.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Version),
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
                  &CommandsApi::Incr),
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
                  &CommandsApi::Decr),
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
                  &CommandsApi::Prepend),
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
                  &CommandsApi::Append),
    CommandHolder("REPLACE",
                  "<key> <flags> <exptime> <value>",
                  "Store key/value pair in Memcached, "
                  "but only if the server "
                  "already hold data for this key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &CommandsApi::Replace),
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
                  &CommandsApi::Add),
    CommandHolder("SCAN",
                  "<cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate the keys space",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  4,
                  &CommandsApi::Scan),
    CommandHolder("KEYS",
                  "<key_start> <key_end> <limit>",
                  "Find all keys matching the given limits.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Keys),
    CommandHolder("DBKCOUNT",
                  "-",
                  "Return the number of keys in the "
                  "selected database",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::DBkcount),
    CommandHolder("FLUSHDB",
                  "-",
                  "Remove all keys from the current database",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::FlushDB),
    CommandHolder("SELECT",
                  "<name>",
                  "Change the selected database for the "
                  "current connection",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Select),
    CommandHolder("SET",
                  "<key> <value>",
                  "Set the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Set),
    CommandHolder("GET",
                  "<key>",
                  "Get the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Get),
    CommandHolder("RENAME",
                  "<key> <newkey>",
                  "Rename a key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Rename),
    CommandHolder("DEL",
                  "<key> [key ...]",
                  "Delete key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Delete),
    CommandHolder("EXPIRE",
                  "<key> <exptime>",
                  "Set a key's time to live in seconds",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::SetTTL),
    CommandHolder("TTL",
                  "<key>",
                  "Get the time to live for a key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::GetTTL),
    CommandHolder("QUIT",
                  "-",
                  "Close the connection",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Quit)};
}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
