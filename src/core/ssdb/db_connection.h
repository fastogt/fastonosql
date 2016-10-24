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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for int64_t, uint64_t

#include <map>     // for map
#include <string>  // for string
#include <vector>  // for vector

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT

#include "core/db_connection/cdb_connection.h"

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

common::Error CreateConnection(const Config& config, NativeConnection** context);
common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error TestConnection(ConnectionSettings* settings);

class DBConnection : public core::CDBConnection<NativeConnection, Config, SSDB> {
 public:
  typedef core::CDBConnection<NativeConnection, Config, SSDB> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  static const char* VersionApi();

  common::Error Info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error Auth(const std::string& password) WARN_UNUSED_RESULT;
  common::Error Setx(const std::string& key,
                     const std::string& value,
                     ttl_t ttl) WARN_UNUSED_RESULT;
  common::Error Incr(const std::string& key, int64_t incrby, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Scan(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Rscan(const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiGet(const std::vector<std::string>& keys, std::vector<std::string>* ret);
  common::Error MultiSet(const std::map<std::string, std::string>& kvs) WARN_UNUSED_RESULT;
  common::Error MultiDel(const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Hget(const std::string& name,
                     const std::string& key,
                     std::string* val) WARN_UNUSED_RESULT;
  common::Error Hgetall(const std::string& name, std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hset(const std::string& name,
                     const std::string& key,
                     const std::string& val) WARN_UNUSED_RESULT;
  common::Error Hdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error Hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret)
      WARN_UNUSED_RESULT;
  common::Error Hsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Hclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Hkeys(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hscan(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hrscan(const std::string& name,
                       const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiHget(const std::string& name,
                           const std::vector<std::string>& keys,
                           std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiHset(const std::string& name,
                           const std::map<std::string, std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Zget(const std::string& name,
                     const std::string& key,
                     int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zset(const std::string& name,
                     const std::string& key,
                     int64_t score) WARN_UNUSED_RESULT;
  common::Error Zdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error Zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret)
      WARN_UNUSED_RESULT;
  common::Error Zsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrank(const std::string& name,
                      const std::string& key,
                      int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrrank(const std::string& name,
                       const std::string& key,
                       int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrange(const std::string& name,
                       uint64_t offset,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zrrange(const std::string& name,
                        uint64_t offset,
                        uint64_t limit,
                        std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zkeys(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zscan(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zrscan(const std::string& name,
                       const std::string& key_start,
                       int64_t* score_start,
                       int64_t* score_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiZget(const std::string& name,
                           const std::vector<std::string>& keys,
                           std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiZset(const std::string& name, const std::map<std::string, int64_t>& kss);
  common::Error MultiZdel(const std::string& name,
                           const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Qpush(const std::string& name, const std::string& item) WARN_UNUSED_RESULT;
  common::Error Qpop(const std::string& name, std::string* item) WARN_UNUSED_RESULT;
  common::Error Qslice(const std::string& name,
                       int64_t begin,
                       int64_t end,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Qclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error DBsize(int64_t* size) WARN_UNUSED_RESULT;

  // extended api
  common::Error DBkcount(size_t* size) WARN_UNUSED_RESULT;
  common::Error Help(int argc, const char** argv) WARN_UNUSED_RESULT;
  common::Error Flushdb() WARN_UNUSED_RESULT;

 private:
  common::Error SetInner(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error GetInner(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error DelInner(const std::string& key) WARN_UNUSED_RESULT;

  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;

  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) override;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) override;
};

common::Error select(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error get(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error del(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error rename(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error set_ttl(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

common::Error info(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error auth(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error setx(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error incr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error keys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error scan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error rscan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_get(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_set(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_del(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hget(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hgetall(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hset(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hdel(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hincr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hsize(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hclear(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hkeys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hscan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error hrscan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_hget(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_hset(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zget(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zset(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zdel(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zincr(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zsize(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zclear(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zrank(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zrrank(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zrange(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zrrange(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zkeys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zscan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error zrscan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_zget(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_zset(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error multi_zdel(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error qpush(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error qpop(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error qslice(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error qclear(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error dbsize(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

common::Error dbkcount(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
common::Error flushdb(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

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
    CommandHolder("RENAME",
                  "<key> <newkey>",
                  "Rename a key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &rename),
    CommandHolder("SETX",
                  "<key> <value> <ttl>",
                  "Set the value of the key, with a time to live.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &setx),
    CommandHolder("DEL",
                  "<key> [key ...]",
                  "Delete key.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
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
                  "<keys> [keys ...]",
                  "Get the values related to the specified "
                  "multiple keys",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &multi_get),
    CommandHolder("MULTI_SET",
                  "<key> <value> [key value ...]",
                  "Set multiple key-value pairs(kvs) in "
                  "one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &multi_set),
    CommandHolder("MULTI_DEL",
                  "<keys> [keys ...]",
                  "Delete specified multiple keys.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
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
                  "<name> <key> <value> [key value ...]",
                  "Set multiple key-value pairs(kvs) of a "
                  "hashmap in one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
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
                  "<name> <key> <score> [key score ...]",
                  "Set multiple key-score pairs(kvs) of a "
                  "zset in one method call.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
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

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
