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

#include "core/db/ssdb/internal/commands_api.h"

#include "core/db/ssdb/db_connection.h"  // for DBConnection

namespace fastonosql {
namespace core {
namespace ssdb {

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
                                                                  "Return information about the server.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  1,
                                                                  &CommandsApi::Info),
                                                    CommandHolder(DB_SCAN_COMMAND,
                                                                  "<cursor> [MATCH pattern] [COUNT count]",
                                                                  "Incrementally iterate the keys space",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  4,
                                                                  &CommandsApi::Scan),
                                                    CommandHolder("SCANSSDB",
                                                                  "<key_start> <key_end> <limit>",
                                                                  "List keys in range (key_start, key_end].",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::ScanSSDB),
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
                                                                  &CommandsApi::Quit),
                                                    CommandHolder("AUTH",
                                                                  "<password>",
                                                                  "Authenticate to the server",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Auth),
                                                    CommandHolder("SETX",
                                                                  "<key> <value> <ttl>",
                                                                  "Set the value of the key, with a time to live.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Setx),
                                                    CommandHolder("INCR",
                                                                  "<key> [num]",
                                                                  "Increment the number stored at key by num.\n"
                                                                  "The num argument could be a negative integer.\n"
                                                                  "The old number is first converted to an integer "
                                                                  "before "
                                                                  "increment, assuming it "
                                                                  "was stored as literal integer.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  1,
                                                                  &CommandsApi::Incr),
                                                    CommandHolder("RSCAN",
                                                                  "<key_start> <key_end> <limit>",
                                                                  "List keys in range (key_start, key_end].",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Rscan),
                                                    CommandHolder("MULTI_GET",
                                                                  "<keys> [keys ...]",
                                                                  "Get the values related to the specified "
                                                                  "multiple keys",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::MultiGet),
                                                    CommandHolder("MULTI_SET",
                                                                  "<key> <value> [key value ...]",
                                                                  "Set multiple key-value pairs(kvs) in "
                                                                  "one method call.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::MultiSet),
                                                    CommandHolder("MULTI_DEL",
                                                                  "<keys> [keys ...]",
                                                                  "Delete specified multiple keys.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::MultiDel),
                                                    CommandHolder("HSET",
                                                                  "<name> <key> <value>",
                                                                  "Set the string value in argument as "
                                                                  "value of the key of a hashmap.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Hset),
                                                    CommandHolder("HGET",
                                                                  "<name> <key>",
                                                                  "Get the value related to the specified "
                                                                  "key of a hashmap",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Hget),
                                                    CommandHolder("HDEL",
                                                                  "<name> <key>",
                                                                  "Delete specified key of a hashmap.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Hdel),
                                                    CommandHolder("HINCR",
                                                                  "<name> <key> <num>",
                                                                  "Increment the number stored at key in a hashmap "
                                                                  "by num.\n"
                                                                  "The num argument could be a negative integer.\n"
                                                                  "The old number is first converted to an integer "
                                                                  "before "
                                                                  "increment, assuming it "
                                                                  "was stored as literal integer.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Hincr),
                                                    CommandHolder("HSIZE",
                                                                  "<name>",
                                                                  "Return the number of pairs of a hashmap.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Hsize),
                                                    CommandHolder("HCLEAR",
                                                                  "<name>",
                                                                  "Delete all keys in a hashmap.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Hclear),
                                                    CommandHolder("HKEYS",
                                                                  "<name> <key_start> <key_end> <limit>",
                                                                  "List keys of a hashmap in range "
                                                                  "(key_start, key_end].",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  4,
                                                                  0,
                                                                  &CommandsApi::Hkeys),
                                                    CommandHolder("HGETALL",
                                                                  "<name>",
                                                                  "Returns the whole hash, as an array of "
                                                                  "strings indexed by strings.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Hgetall),
                                                    CommandHolder("HSCAN",
                                                                  "<name> <key_start> <key_end> <limit>",
                                                                  "List key-value pairs of a hashmap with "
                                                                  "keys in range "
                                                                  "(key_start, key_end].",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  4,
                                                                  0,
                                                                  &CommandsApi::Hscan),
                                                    CommandHolder("HRSCAN",
                                                                  "<name> <key_start> <key_end> <limit>",
                                                                  "List key-value pairs of a hashmap with "
                                                                  "keys in range "
                                                                  "(key_start, key_end], in "
                                                                  "reverse order.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  4,
                                                                  0,
                                                                  &CommandsApi::Hrscan),
                                                    CommandHolder("MULTI_HGET",
                                                                  "<name> <keys>",
                                                                  "Get the values related to the specified "
                                                                  "multiple keys of a hashmap.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::MultiHget),
                                                    CommandHolder("MULTI_HSET",
                                                                  "<name> <key> <value> [key value ...]",
                                                                  "Set multiple key-value pairs(kvs) of a "
                                                                  "hashmap in one method call.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::MultiHset),
                                                    CommandHolder("ZSET",
                                                                  "<name> <key> <score>",
                                                                  "Set the score of the key of a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  4,
                                                                  0,
                                                                  &CommandsApi::Zset),
                                                    CommandHolder("ZGET",
                                                                  "<name> <key>",
                                                                  "Get the score related to the specified "
                                                                  "key of a zset",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Zget),
                                                    CommandHolder("ZDEL",
                                                                  "<name> <key>",
                                                                  "Delete specified key of a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Zdel),
                                                    CommandHolder("ZINCR",
                                                                  "<name> <key> <num>",
                                                                  "Increment the number stored at key in a zset by "
                                                                  "num.\n"
                                                                  "The num argument could be a negative integer.\n"
                                                                  "The old number is first converted to an integer "
                                                                  "before "
                                                                  "increment, assuming it "
                                                                  "was stored as literal integer.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Zincr),
                                                    CommandHolder("ZSIZE",
                                                                  "<name>",
                                                                  "Return the number of pairs of a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Zsize),
                                                    CommandHolder("ZCLEAR",
                                                                  "<name>",
                                                                  "Delete all keys in a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Zclear),
                                                    CommandHolder("ZRANK",
                                                                  "<name> <key>",
                                                                  "Returns the rank(index) of a given key in the "
                                                                  "specified "
                                                                  "sorted set, starting at "
                                                                  "0 for the item with the smallest score.\n"
                                                                  "zrrank starts at 0 for the item with the largest "
                                                                  "score.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Zrank),
                                                    CommandHolder("ZRRANK",
                                                                  "<name> <key>",
                                                                  "Returns the rank(index) of a given key in the "
                                                                  "specified "
                                                                  "sorted set, starting at "
                                                                  "0 for the item with the smallest score.\n"
                                                                  "zrrank starts at 0 for the item with the largest "
                                                                  "score.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Zrrank),
                                                    CommandHolder("ZRANGE",
                                                                  "<name> <offset> <limit>",
                                                                  "Returns a range of key-score pairs by "
                                                                  "index range [offset, "
                                                                  "offset + limit). "
                                                                  "Zrrange iterates in reverse order.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Zrange),
                                                    CommandHolder("ZRRANGE",
                                                                  "<name> <offset> <limit>",
                                                                  "Returns a range of key-score pairs by "
                                                                  "index range [offset, "
                                                                  "offset + limit). "
                                                                  "Zrrange iterates in reverse order.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Zrrange),
                                                    CommandHolder("ZKEYS",
                                                                  "<name> <key_start> <score_start> "
                                                                  "<score_end> <limit>",
                                                                  "List keys in a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  4,
                                                                  0,
                                                                  &CommandsApi::Zkeys),
                                                    CommandHolder("ZSCAN",
                                                                  "<name> <key_start> <score_start> "
                                                                  "<score_end> <limit>",
                                                                  "List key-score pairs in a zset, where "
                                                                  "key-score in range "
                                                                  "(key_start+score_start, "
                                                                  "score_end].\n"
                                                                  "If key_start is empty, keys with a "
                                                                  "score greater than or equal to "
                                                                  "score_start will be "
                                                                  "returned.\n"
                                                                  "If key_start is not empty, keys with "
                                                                  "score larger than score_start, "
                                                                  "and keys larger than key_start also "
                                                                  "with score equal to score_start "
                                                                  "will be returned.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  5,
                                                                  0,
                                                                  &CommandsApi::Zscan),
                                                    CommandHolder("ZRSCAN",
                                                                  "<name> <key_start> <score_start> <score_end> "
                                                                  "<limit>",
                                                                  "List key-score pairs of a zset, in reverse order.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  5,
                                                                  0,
                                                                  &CommandsApi::Zrscan),
                                                    CommandHolder("MULTI_ZGET",
                                                                  "<name> <keys>",
                                                                  "Get the values related to the specified "
                                                                  "multiple keys of a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::MultiZget),
                                                    CommandHolder("MULTI_ZSET",
                                                                  "<name> <key> <score> [key score ...]",
                                                                  "Set multiple key-score pairs(kvs) of a "
                                                                  "zset in one method call.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  INFINITE_COMMAND_ARGS,
                                                                  &CommandsApi::MultiZset),
                                                    CommandHolder("MULTI_ZDEL",
                                                                  "<name> <keys>",
                                                                  "Delete specified multiple keys of a zset.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::MultiZdel),
                                                    CommandHolder("QPUSH",
                                                                  "<name> <item>",
                                                                  "Adds an or more than one element to the "
                                                                  "end of the queue.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Qpush),
                                                    CommandHolder("QPOP",
                                                                  "<name> <size>",
                                                                  "Pop out one or more elements from the "
                                                                  "head of a queue.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  2,
                                                                  0,
                                                                  &CommandsApi::Qpop),
                                                    CommandHolder("QSLICE",
                                                                  "<name> <begin> <end>",
                                                                  "Returns a portion of elements from the "
                                                                  "queue at the "
                                                                  "specified range [begin, "
                                                                  "end]. begin and end could be negative.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  3,
                                                                  0,
                                                                  &CommandsApi::Qslice),
                                                    CommandHolder("QCLEAR",
                                                                  "<name>",
                                                                  "Clear the queue.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  1,
                                                                  0,
                                                                  &CommandsApi::Qclear),
                                                    CommandHolder("DBSIZE",
                                                                  "-",
                                                                  "Return the approximate size of the database, in "
                                                                  "bytes. If "
                                                                  "compression is "
                                                                  "enabled, the size will be of the compressed data.",
                                                                  UNDEFINED_SINCE,
                                                                  UNDEFINED_EXAMPLE_STR,
                                                                  0,
                                                                  0,
                                                                  &CommandsApi::DBsize)};

common::Error CommandsApi::Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = ssdb->Info(argv.size() == 1 ? argv[0] : std::string(), &statsout);
  if (err) {
    return err;
  }

  ServerInfo sinf(statsout);
  common::StringValue* val = common::Value::CreateStringValue(sinf.ToString());
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::ScanSSDB(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_inval();
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->ScanSsdb(argv[0], argv[1], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::DBsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t dbsize = 0;
  common::Error err = ssdb->DBsize(&dbsize);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(dbsize);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Auth(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Auth(argv[0]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Setx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ttl_t ttl;
  if (!common::ConvertFromString(argv[2], &ttl)) {
    return common::make_error_inval();
  }
  common::Error err = ssdb->Setx(argv[0], argv[1], ttl);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[1], &incrby)) {
    return common::make_error_inval();
  }
  int64_t ret = 0;
  common::Error err = ssdb->Incr(argv[0], incrby, &ret);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Rscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_inval();
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Rscan(argv[0], argv[1], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 0; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->MultiGet(keysget, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keysset;
  for (size_t i = 0; i < argv.size(); i += 2) {
    keysset[argv[i]] = argv[i + 1];
  }

  common::Error err = ssdb->MultiSet(keysset);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiDel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 0; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error err = ssdb->MultiDel(keysget);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error err = ssdb->Hget(argv[0], argv[1], &ret);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hgetall(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hgetall(argv[0], &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Hset(argv[0], argv[1], argv[2]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Hdel(argv[0], argv[1]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hincr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[2], &incrby)) {
    return common::make_error_inval();
  }
  int64_t res = 0;
  common::Error err = ssdb->Hincr(argv[0], argv[1], incrby, &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Hsize(argv[0], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Hclear(argv[0], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_inval();
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hkeys(argv[0], argv[1], argv[2], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_inval();
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hscan(argv[0], argv[1], argv[2], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hrscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_inval();
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hrscan(argv[0], argv[1], argv[2], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiHget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 1; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->MultiHget(argv[0], keysget, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiHset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keys;
  for (size_t i = 1; i < argv.size(); i += 2) {
    keys[argv[i]] = argv[i + 1];
  }

  common::Error err = ssdb->MultiHset(argv[0], keys);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret;
  common::Error err = ssdb->Zget(argv[0], argv[1], &ret);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t score;
  if (!common::ConvertFromString(argv[2], &score)) {
    return common::make_error_inval();
  }

  common::Error err = ssdb->Zset(argv[0], argv[1], score);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Zdel(argv[0], argv[1]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zincr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[2], &incrby)) {
    return common::make_error_inval();
  }

  int64_t ret = 0;
  common::Error err = ssdb->Zincr(argv[0], argv[1], incrby, &ret);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zsize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zsize(argv[0], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zclear(argv[0], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zrank(argv[0], argv[1], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zrrank(argv[0], argv[1], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t offset;
  if (!common::ConvertFromString(argv[1], &offset)) {
    return common::make_error_inval();
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_inval();
  }

  std::vector<std::string> res;
  common::Error err = ssdb->Zrange(argv[0], offset, limit, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t offset;
  if (!common::ConvertFromString(argv[1], &offset)) {
    return common::make_error_inval();
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_inval();
  }

  std::vector<std::string> res;
  common::Error err = ssdb->Zrrange(argv[0], offset, limit, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_inval();
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_inval();
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_inval();
  }

  common::Error err = ssdb->Zkeys(argv[0], argv[1], &st, &end, limit, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_inval();
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_inval();
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_inval();
  }
  common::Error err = ssdb->Zscan(argv[0], argv[1], &st, &end, limit, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_inval();
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_inval();
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_inval();
  }
  common::Error err = ssdb->Zrscan(argv[0], argv[1], &st, &end, limit, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 1; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> res;
  common::Error err = ssdb->MultiZget(argv[0], keysget, &res);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, int64_t> keysget;
  for (size_t i = 1; i < argv.size(); i += 2) {
    int64_t val;
    if (!common::ConvertFromString(argv[i + 1], &val)) {
      keysget[argv[i]] = val;
    }
  }

  common::Error err = ssdb->MultiZset(argv[0], keysget);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 1; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error err = ssdb->MultiZdel(argv[0], keysget);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Qpush(argv[0], argv[1]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error err = ssdb->Qpop(argv[0], &ret);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qslice(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t begin;
  if (!common::ConvertFromString(argv[1], &begin)) {
    return common::make_error_inval();
  }

  int64_t end;
  if (!common::ConvertFromString(argv[2], &end)) {
    return common::make_error_inval();
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->Qslice(argv[0], begin, end, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qclear(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Qclear(argv[0], &res);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
