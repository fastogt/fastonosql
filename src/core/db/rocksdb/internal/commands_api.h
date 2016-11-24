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
namespace rocksdb {

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

common::Error info(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error mget(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error merge(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out);
common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);
common::Error scan(internal::CommandHandler* handler,
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
common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out);
common::Error quit(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out);

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
                  "<key> [key ...]",
                  "Get the value of a key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
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
                  "<key> [key ...]",
                  "Delete key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &del),
    CommandHolder("RENAME",
                  "<key> <newkey>",
                  "Rename a key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &rename),
    CommandHolder("KEYS",
                  "<key_start> <key_end> <limit>",
                  "Find all keys matching the given limits.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &keys),
    CommandHolder("SCAN",
                  "<cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate the keys space",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  4,
                  &scan),
    CommandHolder("INFO",
                  "<args>",
                  "These command return database information.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &info),
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
