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

#include "core/db/rocksdb/internal/commands_api.h"

#include "core/db/rocksdb/db_connection.h"

namespace fastonosql {
namespace core {
namespace rocksdb {

const internal::ConstantCommandsArray g_commands = {CommandHolder(DB_HELP_COMMAND,
                                                                  "[command]",
                                                                  "Return how to use command",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  &CommandsApi::Help),
                                                    CommandHolder(DB_INFO_COMMAND,
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
                                                    CommandHolder(DB_DBKCOUNT_COMMAND,
                                                                  "-",
                                                                  "Return the number of keys in the "
                                                                  "selected database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  &CommandsApi::DBkcount),
                                                    CommandHolder(DB_FLUSHDB_COMMAND,
                                                                  "-",
                                                                  "Remove all keys from the current database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  &CommandsApi::FlushDB),
                                                    CommandHolder(DB_SELECTDB_COMMAND,
                                                                  "<name>",
                                                                  "Change the selected database for the "
                                                                  "current connection",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Select),
                                                    CommandHolder(DB_SET_KEY_COMMAND,
                                                                  "<key> <value>",
                                                                  "Set the value of a key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Set),
                                                    CommandHolder(DB_GET_KEY_COMMAND,
                                                                  "<key>",
                                                                  "Get the value of a key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Get),
                                                    CommandHolder(DB_RENAME_KEY_COMMAND,
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
                                                    CommandHolder(DB_DELETE_KEY_COMMAND,
                                                                  "<key> [key ...]",
                                                                  "Delete key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::Delete),
                                                    CommandHolder(DB_QUIT_COMMAND,
                                                                  "-",
                                                                  "Close the connection",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  &CommandsApi::Quit)};

common::Error CommandsApi::Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = rocks->Info(argv.size() == 1 ? argv[0] : std::string(), &statsout);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ServerInfo(statsout).ToString());
  FastoObject* child = new FastoObject(out, val, rocks->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 0; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = rocks->Mget(keysget, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Merge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->Merge(argv[0], argv[1]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
