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

#include "core/db/lmdb/internal/commands_api.h"

#include "core/db/lmdb/db_connection.h"

namespace fastonosql {
namespace core {
namespace lmdb {

const internal::ConstantCommandsArray g_commands = {CommandHolder(DB_HELP_COMMAND,
                                                                  "[command]",
                                                                  "Return how to use command",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Help),
                                                    CommandHolder(DB_INFO_COMMAND,
                                                                  "[section]",
                                                                  "These command return database information.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Info),
                                                    CommandHolder("CONFIG GET",
                                                                  "<parameter>",
                                                                  "Get the value of a configuration parameter",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::ConfigGet),
                                                    CommandHolder(DB_CREATEDB_COMMAND,
                                                                  "<name>",
                                                                  "Create database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::CreateDatabase),
                                                    CommandHolder(DB_REMOVEDB_COMMAND,
                                                                  "<name>",
                                                                  "Remove database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::RemoveDatabase),
                                                    CommandHolder(DB_SCAN_COMMAND,
                                                                  "<cursor> [MATCH pattern] [COUNT count]",
                                                                  "Incrementally iterate the keys space",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  4,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Scan),
                                                    CommandHolder(DB_KEYS_COMMAND,
                                                                  "<key_start> <key_end> <limit>",
                                                                  "Find all keys matching the given limits.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Keys),
                                                    CommandHolder(DB_DBKCOUNT_COMMAND,
                                                                  "-",
                                                                  "Return the number of keys in the "
                                                                  "selected database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::DBkcount),
                                                    CommandHolder(DB_FLUSHDB_COMMAND,
                                                                  "-",
                                                                  "Remove all keys from the current database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::FlushDB),
                                                    CommandHolder(LMDB_DROPDB_COMMAND,
                                                                  "-",
                                                                  "Drop database",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::DropDatabase),
                                                    CommandHolder(DB_SELECTDB_COMMAND,
                                                                  "<name>",
                                                                  "Change the selected database for the "
                                                                  "current connection",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Select),
                                                    CommandHolder(DB_SET_KEY_COMMAND,
                                                                  "<key> <value>",
                                                                  "Set the value of a key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Set),
                                                    CommandHolder(DB_GET_KEY_COMMAND,
                                                                  "<key>",
                                                                  "Get the value of a key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Get),
                                                    CommandHolder(DB_RENAME_KEY_COMMAND,
                                                                  "<key> <newkey>",
                                                                  "Rename a key",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Rename),
                                                    CommandHolder(DB_DELETE_KEY_COMMAND,
                                                                  "<key> [key ...]",
                                                                  "Delete key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Delete),
                                                    CommandHolder(DB_QUIT_COMMAND,
                                                                  "-",
                                                                  "Close the connection",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  CommandInfo::Native,
                                                                  &CommandsApi::Quit)};

common::Error CommandsApi::Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* mdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = mdb->Info(argv.size() == 1 ? argv[0] : std::string(), &statsout);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ServerInfo(statsout).ToString());
  FastoObject* child = new FastoObject(out, val, mdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::ConfigGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* mdb = static_cast<DBConnection*>(handler);
  if (argv[0] != "databases") {
    return common::make_error_inval();
  }

  std::vector<std::string> dbs;
  common::Error err = mdb->ConfigGetDatabases(&dbs);
  if (err) {
    return err;
  }

  common::ArrayValue* arr = new common::ArrayValue;
  arr->AppendStrings(dbs);
  FastoObject* child = new FastoObject(out, arr, mdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::DropDatabase(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* mdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = mdb->DropDatabase();
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
