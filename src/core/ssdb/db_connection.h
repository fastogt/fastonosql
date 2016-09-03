/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for int64_t, uint64_t

#include <map>     // for map
#include <string>  // for string
#include <vector>  // for vector

#include "common/error.h"   // for Error
#include "common/macros.h"  // for WARN_UNUSED_RESULT

#include "core/command_handler.h"
#include "core/db_connection.h"

#include "core/ssdb/config.h"
#include "core/ssdb/connection_settings.h"
#include "core/ssdb/server_info.h"

namespace ssdb {
class Client;
}

namespace fastonosql {
namespace core {
namespace ssdb {

typedef ::ssdb::Client NativeConnection;

common::Error createConnection(const Config& config, NativeConnection** context);
common::Error createConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error testConnection(ConnectionSettings* settings);

class DBConnection : public core::DBConnection<NativeConnection, Config, SSDB>,
                     public CommandHandler {
 public:
  typedef core::DBConnection<NativeConnection, Config, SSDB> base_class;
  DBConnection();

  static const char* versionApi();

  common::Error info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error auth(const std::string& password) WARN_UNUSED_RESULT;
  common::Error get(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error set(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error setx(const std::string& key, const std::string& value, int ttl) WARN_UNUSED_RESULT;
  common::Error del(const std::string& key) WARN_UNUSED_RESULT;
  common::Error incr(const std::string& key, int64_t incrby, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error scan(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error rscan(const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error multi_get(const std::vector<std::string>& keys, std::vector<std::string>* ret);
  common::Error multi_set(const std::map<std::string, std::string>& kvs) WARN_UNUSED_RESULT;
  common::Error multi_del(const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error hget(const std::string& name,
                     const std::string& key,
                     std::string* val) WARN_UNUSED_RESULT;
  common::Error hgetall(const std::string& name, std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error hset(const std::string& name,
                     const std::string& key,
                     const std::string& val) WARN_UNUSED_RESULT;
  common::Error hdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret)
      WARN_UNUSED_RESULT;
  common::Error hsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error hclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error hkeys(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error hscan(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error hrscan(const std::string& name,
                       const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error multi_hget(const std::string& name,
                           const std::vector<std::string>& keys,
                           std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error multi_hset(const std::string& name,
                           const std::map<std::string, std::string>& keys) WARN_UNUSED_RESULT;
  common::Error zget(const std::string& name,
                     const std::string& key,
                     int64_t* ret) WARN_UNUSED_RESULT;
  common::Error zset(const std::string& name,
                     const std::string& key,
                     int64_t score) WARN_UNUSED_RESULT;
  common::Error zdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret)
      WARN_UNUSED_RESULT;
  common::Error zsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error zclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error zrank(const std::string& name,
                      const std::string& key,
                      int64_t* ret) WARN_UNUSED_RESULT;
  common::Error zrrank(const std::string& name,
                       const std::string& key,
                       int64_t* ret) WARN_UNUSED_RESULT;
  common::Error zrange(const std::string& name,
                       uint64_t offset,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error zrrange(const std::string& name,
                        uint64_t offset,
                        uint64_t limit,
                        std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error zkeys(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error zscan(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error zrscan(const std::string& name,
                       const std::string& key_start,
                       int64_t* score_start,
                       int64_t* score_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error multi_zget(const std::string& name,
                           const std::vector<std::string>& keys,
                           std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error multi_zset(const std::string& name, const std::map<std::string, int64_t>& kss);
  common::Error multi_zdel(const std::string& name,
                           const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error qpush(const std::string& name, const std::string& item) WARN_UNUSED_RESULT;
  common::Error qpop(const std::string& name, std::string* item) WARN_UNUSED_RESULT;
  common::Error qslice(const std::string& name,
                       int64_t begin,
                       int64_t end,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error qclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error dbsize(int64_t* size) WARN_UNUSED_RESULT;

  // extended api
  common::Error dbkcount(size_t* size) WARN_UNUSED_RESULT;
  common::Error help(int argc, char** argv) WARN_UNUSED_RESULT;
  common::Error flushdb() WARN_UNUSED_RESULT;
};

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error auth(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error setx(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error incr(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error scan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error rscan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_get(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_set(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_del(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hgetall(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hset(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hdel(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hincr(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hsize(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hclear(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hkeys(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hscan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error hrscan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_hget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_hset(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zset(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zdel(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zincr(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zsize(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zclear(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zrank(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zrrank(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zrange(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zrrange(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zkeys(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zscan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error zrscan(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_zget(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_zset(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error multi_zdel(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error qpush(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error qpop(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error qslice(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error qclear(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out);

common::Error dbkcount(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out);

// TODO: SETNX command imlementation
static const std::vector<CommandHolder> ssdbCommands = {
    CommandHolder("AUTH",
                  "<password>",
                  "Authenticate to the server",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &auth),
    CommandHolder("SET",
                  "<key> <value>",
                  "Set the value of the key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &set),
    CommandHolder("GET",
                  "<key>",
                  "Get the value related to the specified key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &get),
    CommandHolder("SETX",
                  "<key> <value> <ttl>",
                  "Set the value of the key, with a time to live.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &setx),
    CommandHolder("DEL",
                  "<key>",
                  "Delete specified key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &del),
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
                  &incr),
    CommandHolder("KEYS",
                  "<key_start> <key_end> <limit>",
                  "List keys in range (key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &keys),
    CommandHolder("SCAN",
                  "<key_start> <key_end> <limit>",
                  "List keys in range (key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &scan),
    CommandHolder("RSCAN",
                  "<key_start> <key_end> <limit>",
                  "List keys in range (key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &rscan),
    CommandHolder("MULTI_GET",
                  "<keys>",
                  "Get the values related to the specified "
                  "multiple keys",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &multi_get),
    CommandHolder("MULTI_SET",
                  "<kvs>",
                  "Set multiple key-value pairs(kvs) in "
                  "one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &multi_set),
    CommandHolder("MULTI_DEL",
                  "<keys>",
                  "Delete specified multiple keys.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &multi_del),
    CommandHolder("HSET",
                  "<name> <key> <value>",
                  "Set the string value in argument as "
                  "value of the key of a hashmap.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &hset),
    CommandHolder("HGET",
                  "<name> <key>",
                  "Get the value related to the specified "
                  "key of a hashmap",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &hget),
    CommandHolder("HDEL",
                  "<name> <key>",
                  "Delete specified key of a hashmap.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &hdel),
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
                  &hincr),
    CommandHolder("HSIZE",
                  "<name>",
                  "Return the number of pairs of a hashmap.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &hsize),
    CommandHolder("HCLEAR",
                  "<name>",
                  "Delete all keys in a hashmap.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &hclear),
    CommandHolder("HKEYS",
                  "<name> <key_start> <key_end> <limit>",
                  "List keys of a hashmap in range "
                  "(key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &hkeys),
    CommandHolder("HGETALL",
                  "<name>",
                  "Returns the whole hash, as an array of "
                  "strings indexed by strings.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &hgetall),
    CommandHolder("HSCAN",
                  "<name> <key_start> <key_end> <limit>",
                  "List key-value pairs of a hashmap with "
                  "keys in range "
                  "(key_start, key_end].",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &hscan),
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
                  &hrscan),
    CommandHolder("MULTI_HGET",
                  "<name> <keys>",
                  "Get the values related to the specified "
                  "multiple keys of a hashmap.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &multi_hget),
    CommandHolder("MULTI_HSET",
                  "<name> <kvs>",
                  "Set multiple key-value pairs(kvs) of a "
                  "hashmap in one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &multi_hset),
    CommandHolder("ZSET",
                  "<name> <key> <score>",
                  "Set the score of the key of a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &zset),
    CommandHolder("ZGET",
                  "<name> <key>",
                  "Get the score related to the specified "
                  "key of a zset",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &zget),
    CommandHolder("ZDEL",
                  "<name> <key>",
                  "Delete specified key of a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &zdel),
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
                  &zincr),
    CommandHolder("ZSIZE",
                  "<name>",
                  "Return the number of pairs of a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &zsize),
    CommandHolder("ZCLEAR",
                  "<name>",
                  "Delete all keys in a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &zclear),
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
                  &zrank),
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
                  &zrrank),
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
                  &zrange),
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
                  &zrrange),
    CommandHolder("ZKEYS",
                  "<name> <key_start> <score_start> "
                  "<score_end> <limit>",
                  "List keys in a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &zkeys),
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
                  &zscan),
    CommandHolder("ZRSCAN",
                  "<name> <key_start> <score_start> <score_end> "
                  "<limit>",
                  "List key-score pairs of a zset, in reverse order.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  5,
                  0,
                  &zrscan),
    CommandHolder("MULTI_ZGET",
                  "<name> <keys>",
                  "Get the values related to the specified "
                  "multiple keys of a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &multi_zget),
    CommandHolder("MULTI_ZSET",
                  "<name> <kvs>",
                  "Set multiple key-score pairs(kvs) of a "
                  "zset in one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &multi_zset),
    CommandHolder("MULTI_ZDEL",
                  "<name> <keys>",
                  "Delete specified multiple keys of a zset.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &multi_zdel),
    CommandHolder("INFO",
                  "[opt]",
                  "Return information about the server.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &info),
    CommandHolder("QPUSH",
                  "<name> <item>",
                  "Adds an or more than one element to the "
                  "end of the queue.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &qpush),
    CommandHolder("QPOP",
                  "<name> <size>",
                  "Pop out one or more elements from the "
                  "head of a queue.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &qpop),
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
                  &qslice),
    CommandHolder("QCLEAR",
                  "<name>",
                  "Clear the queue.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &qclear),
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
                  &dbsize),

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
                  "<command>",
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

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
