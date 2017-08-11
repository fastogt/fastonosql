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

#pragma once

#include <vector>  // for vector

#include <common/error.h>  // for Error

#include "core/command_holder.h"         // for CommandHolder, etc
#include "core/command_info.h"           // for UNDEFINED_EXAMPLE_STR, etc
#include "core/internal/commands_api.h"  // for ApiTraits

namespace fastonosql {
namespace core {
class FastoObject;
}
}  // namespace fastonosql
namespace fastonosql {
namespace core {
namespace internal {
class CommandHandler;
}
}  // namespace core
}  // namespace fastonosql
namespace fastonosql {
namespace core {
namespace rocksdb {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace core {
namespace rocksdb {

struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Merge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
};

static const internal::ConstantCommandsArray g_commands = {CommandHolder("HELP",
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
                                                           CommandHolder("MGET",
                                                                         "<key> [key ...]",
                                                                         "Get the value of a key.",
                                                                         UNDEFINED_SINCE,
                                                                         UNDEFINED_EXAMPLE_STR,
                                                                         1,
                                                                         INFINITE_COMMAND_ARGS,
                                                                         &CommandsApi::Mget),
                                                           CommandHolder("MERGE",
                                                                         "<key> <value>",
                                                                         "Merge the database entry for \"key\" "
                                                                         "with \"value\"",
                                                                         UNDEFINED_SINCE,
                                                                         UNDEFINED_EXAMPLE_STR,
                                                                         2,
                                                                         0,
                                                                         &CommandsApi::Merge),
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

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
