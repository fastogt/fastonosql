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

#include "core/db/memcached/internal/commands_api.h"

#include "core/db/memcached/db_connection.h"

namespace fastonosql {
namespace core {
namespace memcached {

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
                                                    CommandHolder(DB_SCAN_COMMAND,
                                                                  "<cursor> [MATCH pattern] [COUNT count]",
                                                                  "Incrementally iterate the keys space",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  4,
                                                                  &CommandsApi::Scan),
                                                    CommandHolder(DB_KEYS_COMMAND,
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
                                                    CommandHolder(DB_DELETE_KEY_COMMAND,
                                                                  "<key> [key ...]",
                                                                  "Delete key.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::Delete),
                                                    CommandHolder(DB_SET_TTL_COMMAND,
                                                                  "<key> <exptime>",
                                                                  "Set a key's time to live in seconds",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::SetTTL),
                                                    CommandHolder(DB_GET_TTL_COMMAND,
                                                                  "<key>",
                                                                  "Get the time to live for a key",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::GetTTL),
                                                    CommandHolder(DB_QUIT_COMMAND,
                                                                  "-",
                                                                  "Close the connection",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  &CommandsApi::Quit)};

const internal::ConstantCommandsArray g_extended_commands = {};

common::Error CommandsApi::Version(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  UNUSED(out);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  return mem->VersionServer();
}

common::Error CommandsApi::Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* mem = static_cast<DBConnection*>(handler);
  std::string args = argv.size() == 1 ? argv[0] : std::string();
  if (args.empty() && strcasecmp(args.c_str(), "items") == 0) {
    commands_args_t largv = {"a", "z", "100"};
    return Keys(handler, largv, out);
  }

  ServerInfo::Stats statsout;
  common::Error err = mem->Info(args, &statsout);
  if (err) {
    return err;
  }

  ServerInfo minf(statsout);
  common::StringValue* val = common::Value::CreateStringValue(minf.ToString());
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Add(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_inval();
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_inval();
  }

  common::Error err = mem->AddIfNotExist(key, argv[3], expiration, flags);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Replace(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_inval();
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_inval();
  }

  common::Error err = mem->Replace(key, argv[3], expiration, flags);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Append(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_inval();
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_inval();
  }
  common::Error err = mem->Append(key, argv[3], expiration, flags);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Prepend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_inval();
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_inval();
  }
  common::Error err = mem->Prepend(key, argv[3], expiration, flags);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  uint32_t value;
  if (!common::ConvertFromString(argv[1], &value)) {
    return common::make_error_inval();
  }
  uint64_t result = 0;
  common::Error err = mem->Incr(key, value, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateULongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Decr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  uint32_t value;
  if (!common::ConvertFromString(argv[1], &value)) {
    return common::make_error_inval();
  }
  uint64_t result = 0;
  common::Error err = mem->Decr(key, value, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateULongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, mem->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
