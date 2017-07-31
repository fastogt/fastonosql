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
namespace ssdb {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace core {
namespace ssdb {

struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error ScanSSDB(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);

  static common::Error Auth(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Setx(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Incr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Rscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiGet(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiSet(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiDel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hgetall(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Hrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiHget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiHset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zrrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zrrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Zrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiZget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiZset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error MultiZdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Qpush(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Qpop(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Qslice(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Qclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error DBsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out);
};

static const internal::ConstantCommandsArray g_commands = {
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
                  "Return information about the server.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Info),
    CommandHolder("SCAN",
                  "<key_start> <key_end> <limit>",
                  "List keys in range (key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ScanSSDB),
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

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
