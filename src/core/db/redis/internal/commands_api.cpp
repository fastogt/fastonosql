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

#include "core/db/redis/internal/commands_api.h"

#include "core/db/redis/db_connection.h"
#include "core/db/redis/internal/modules.h"

#include "core/value.h"

namespace fastonosql {
namespace core {
namespace {
inline commands_args_t ExpandCommand(std::initializer_list<command_buffer_t> list, commands_args_t argv) {
  argv.insert(argv.begin(), list);
  return argv;
}
}  // namespace
namespace redis {

common::Error CommandsApi::Auth(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->Auth(argv[0]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Lpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  common::ArrayValue* arr = common::Value::CreateArrayValue();
  for (size_t i = 1; i < argv.size(); ++i) {
    arr->AppendString(argv[i]);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long list_len = 0;
  common::Error err = redis->Lpush(key, NValue(arr), &list_len);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(list_len);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Lrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  int start;
  if (!common::ConvertFromString(argv[1], &start)) {
    return common::make_error_inval();
  }

  int stop;
  if (!common::ConvertFromString(argv[2], &stop)) {
    return common::make_error_inval();
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Lrange(key, start, stop, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({DB_INFO_COMMAND}, argv), out);
}

common::Error CommandsApi::Append(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"APPEND"}, argv), out);
}

common::Error CommandsApi::BgRewriteAof(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BGREWRITEAOF"}, argv), out);
}

common::Error CommandsApi::BgSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BGSAVE"}, argv), out);
}

common::Error CommandsApi::BitCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BITCOUNT"}, argv), out);
}

common::Error CommandsApi::BitField(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BITFIELD"}, argv), out);
}

common::Error CommandsApi::BitOp(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BITOP"}, argv), out);
}

common::Error CommandsApi::BitPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BITPOS"}, argv), out);
}

common::Error CommandsApi::BlPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BLPOP"}, argv), out);
}

common::Error CommandsApi::BrPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BRPOP"}, argv), out);
}

common::Error CommandsApi::BrPopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"BRPOPLPUSH"}, argv), out);
}

common::Error CommandsApi::ClientGetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "GETNAME"}, argv), out);
}

common::Error CommandsApi::ClientKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "KILL"}, argv), out);
}

common::Error CommandsApi::ClientList(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "LIST"}, argv), out);
}

common::Error CommandsApi::ClientPause(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "PAUSE"}, argv), out);
}

common::Error CommandsApi::ClientReply(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "REPLY"}, argv), out);
}

common::Error CommandsApi::ClientSetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "SETNAME"}, argv), out);
}

common::Error CommandsApi::ClusterAddSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "ADDSLOTS"}, argv), out);
}

common::Error CommandsApi::ClusterCountFailureReports(internal::CommandHandler* handler,
                                                      commands_args_t argv,
                                                      FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "COUNT-FAILURE-REPORTS"}, argv), out);
}

common::Error CommandsApi::ClusterCountKeysSinSlot(internal::CommandHandler* handler,
                                                   commands_args_t argv,
                                                   FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLIENT", "COUNTKEYSINSLOT"}, argv), out);
}

common::Error CommandsApi::ClusterDelSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "DELSLOTS"}, argv), out);
}

common::Error CommandsApi::ClusterFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "FAILOVER"}, argv), out);
}

common::Error CommandsApi::ClusterForget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "FORGET"}, argv), out);
}

common::Error CommandsApi::ClusterGetKeySinSlot(internal::CommandHandler* handler,
                                                commands_args_t argv,
                                                FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "GETKEYSINSLOT"}, argv), out);
}

common::Error CommandsApi::ClusterInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "INFO"}, argv), out);
}

common::Error CommandsApi::ClusterKeySlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "KEYSLOT"}, argv), out);
}

common::Error CommandsApi::ClusterMeet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "MEET"}, argv), out);
}

common::Error CommandsApi::ClusterNodes(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "NODES"}, argv), out);
}

common::Error CommandsApi::ClusterReplicate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "REPLICATE"}, argv), out);
}

common::Error CommandsApi::ClusterReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "RESET"}, argv), out);
}

common::Error CommandsApi::ClusterSaveConfig(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "SAVECONFIG"}, argv), out);
}

common::Error CommandsApi::ClusterSetConfigEpoch(internal::CommandHandler* handler,
                                                 commands_args_t argv,
                                                 FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "SET-CONFIG-EPOCH"}, argv), out);
}

common::Error CommandsApi::ClusterSetSlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "SETSLOT"}, argv), out);
}

common::Error CommandsApi::ClusterSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "SLAVES"}, argv), out);
}

common::Error CommandsApi::ClusterSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CLUSTER", "SLOTS"}, argv), out);
}

common::Error CommandsApi::CommandCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"COMMAND", "COUNT"}, argv), out);
}

common::Error CommandsApi::CommandGetKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"COMMAND", "GETKEYS"}, argv), out);
}

common::Error CommandsApi::CommandInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"COMMAND", "INFO"}, argv), out);
}

common::Error CommandsApi::Command(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"COMMAND"}, argv), out);
}

common::Error CommandsApi::ConfigGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CONFIG", "GET"}, argv), out);
}

common::Error CommandsApi::ConfigResetStat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CONFIG", "RESETSTAT"}, argv), out);
}

common::Error CommandsApi::ConfigRewrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CONFIG", "REWRITE"}, argv), out);
}

common::Error CommandsApi::ConfigSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CONFIG", "SET"}, argv), out);
}

common::Error CommandsApi::DbSize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"CONFIG", "DBSIZE"}, argv), out);
}

common::Error CommandsApi::DebugObject(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"DEBUG", "OBJECT"}, argv), out);
}

common::Error CommandsApi::DebugSegFault(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"DEBUG", "SEGFAULT"}, argv), out);
}

common::Error CommandsApi::Discard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);

  return red->CommonExec(ExpandCommand({"DISCARD"}, argv), out);
}

common::Error CommandsApi::Dump(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"DUMP"}, argv), out);
}

common::Error CommandsApi::Echo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ECHO"}, argv), out);
}

common::Error CommandsApi::Eval(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"EVAL"}, argv), out);
}

common::Error CommandsApi::EvalSha(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"EVALSHA"}, argv), out);
}

common::Error CommandsApi::Exec(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"EXEC"}, argv), out);
}

common::Error CommandsApi::Exists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"EXISTS"}, argv), out);
}

common::Error CommandsApi::ExpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"EXPIREAT"}, argv), out);
}

common::Error CommandsApi::FlushALL(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"FLUSHALL"}, argv), out);
}

common::Error CommandsApi::GeoAdd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEOADD"}, argv), out);
}

common::Error CommandsApi::GeoDist(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEODIST"}, argv), out);
}

common::Error CommandsApi::GeoHash(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEOHASH"}, argv), out);
}

common::Error CommandsApi::GeoPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEOPOS"}, argv), out);
}

common::Error CommandsApi::GeoRadius(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEORADIUS"}, argv), out);
}

common::Error CommandsApi::GeoRadiusByMember(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEORADIUSBYMEMBER"}, argv), out);
}

common::Error CommandsApi::GetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GETBIT"}, argv), out);
}

common::Error CommandsApi::GetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GETRANGE"}, argv), out);
}

common::Error CommandsApi::GetSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GETSET"}, argv), out);
}

common::Error CommandsApi::Hdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HDEL"}, argv), out);
}

common::Error CommandsApi::Hexists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HEXISTS"}, argv), out);
}

common::Error CommandsApi::Hget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HGET"}, argv), out);
}

common::Error CommandsApi::HincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HINCRBY"}, argv), out);
}

common::Error CommandsApi::HincrByFloat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HINCRBYFLOAT"}, argv), out);
}

common::Error CommandsApi::Hkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HKEYS"}, argv), out);
}

common::Error CommandsApi::Hlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HLEN"}, argv), out);
}

common::Error CommandsApi::Hmget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HMGET"}, argv), out);
}

common::Error CommandsApi::Hscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HSCAN"}, argv), out);
}

common::Error CommandsApi::Hset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HSET"}, argv), out);
}

common::Error CommandsApi::HsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HSETNX"}, argv), out);
}

common::Error CommandsApi::Hstrlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HSTRLEN"}, argv), out);
}

common::Error CommandsApi::Hvals(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"HVALS"}, argv), out);
}

common::Error CommandsApi::RKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({DB_KEYS_COMMAND}, argv), out);
}

common::Error CommandsApi::LastSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LASTSAVE"}, argv), out);
}

common::Error CommandsApi::Lindex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LINDEX"}, argv), out);
}

common::Error CommandsApi::Linsert(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LINSERT"}, argv), out);
}

common::Error CommandsApi::Llen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LLEN"}, argv), out);
}

common::Error CommandsApi::Lpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LPOP"}, argv), out);
}

common::Error CommandsApi::LpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LPUSHX"}, argv), out);
}

common::Error CommandsApi::Lrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LREM"}, argv), out);
}

common::Error CommandsApi::Lset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LSET"}, argv), out);
}

common::Error CommandsApi::Ltrim(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LTRIM"}, argv), out);
}

common::Error CommandsApi::Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MGET"}, argv), out);
}

common::Error CommandsApi::Migrate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MIGRATE"}, argv), out);
}

common::Error CommandsApi::Move(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MOVE"}, argv), out);
}

common::Error CommandsApi::Mset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MSET"}, argv), out);
}

common::Error CommandsApi::MsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MSETNX"}, argv), out);
}

common::Error CommandsApi::Multi(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MULTI"}, argv), out);
}

common::Error CommandsApi::Object(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"OBJECT"}, argv), out);
}

common::Error CommandsApi::Pexpire(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PEXPIRE"}, argv), out);
}

common::Error CommandsApi::PexpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PEXPIREAT"}, argv), out);
}

common::Error CommandsApi::Pfadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PFADD"}, argv), out);
}

common::Error CommandsApi::Pfcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PFCOUNT"}, argv), out);
}

common::Error CommandsApi::Pfmerge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PFMERGE"}, argv), out);
}

common::Error CommandsApi::Ping(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PING"}, argv), out);
}

common::Error CommandsApi::PsetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PSETEX"}, argv), out);
}

common::Error CommandsApi::Pttl(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PTTL"}, argv), out);
}

common::Error CommandsApi::Publish(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({DB_PUBLISH_COMMAND}, argv), out);
}

common::Error CommandsApi::PubSub(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PUBSUB"}, argv), out);
}

common::Error CommandsApi::PunSubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PUNSUBSCRIBE"}, argv), out);
}

common::Error CommandsApi::RandomKey(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RANDOMKEY"}, argv), out);
}

common::Error CommandsApi::ReadOnly(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"READONLY"}, argv), out);
}

common::Error CommandsApi::ReadWrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"READWRITE"}, argv), out);
}

common::Error CommandsApi::RenameNx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RENAMENX"}, argv), out);
}

common::Error CommandsApi::Restore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RESTORE"}, argv), out);
}

common::Error CommandsApi::Role(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ROLE"}, argv), out);
}

common::Error CommandsApi::Rpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RPOP"}, argv), out);
}

common::Error CommandsApi::RpopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RPOPLPUSH"}, argv), out);
}

common::Error CommandsApi::Rpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RPUSH"}, argv), out);
}

common::Error CommandsApi::RpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RPUSHX"}, argv), out);
}

common::Error CommandsApi::Save(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SAVE"}, argv), out);
}

common::Error CommandsApi::Scard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCARD"}, argv), out);
}

common::Error CommandsApi::ScriptDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCRIPT", "DEBUG"}, argv), out);
}

common::Error CommandsApi::ScriptExists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCRIPT", "EXISTS"}, argv), out);
}

common::Error CommandsApi::ScriptFlush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCRIPT", "FLUSH"}, argv), out);
}

common::Error CommandsApi::ScriptKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCRIPT", "KILL"}, argv), out);
}

common::Error CommandsApi::ScriptLoad(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SCRIPT", "LOAD"}, argv), out);
}

common::Error CommandsApi::Sdiff(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SDIFF"}, argv), out);
}

common::Error CommandsApi::SdiffStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SDIFFSTORE"}, argv), out);
}

common::Error CommandsApi::SetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SETBIT"}, argv), out);
}

common::Error CommandsApi::SetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SETRANGE"}, argv), out);
}

common::Error CommandsApi::Shutdown(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SHUTDOWN"}, argv), out);
}

common::Error CommandsApi::Sinter(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SINTER"}, argv), out);
}

common::Error CommandsApi::SinterStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SINTERSTORE"}, argv), out);
}

common::Error CommandsApi::SisMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SISMEMBER"}, argv), out);
}

common::Error CommandsApi::SlaveOf(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SLAVEOF"}, argv), out);
}

common::Error CommandsApi::SlowLog(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SLOWLOG"}, argv), out);
}

common::Error CommandsApi::Smove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SMOVE"}, argv), out);
}

common::Error CommandsApi::Sort(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SORT"}, argv), out);
}

common::Error CommandsApi::Spop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SPOP"}, argv), out);
}

common::Error CommandsApi::SRandMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SRANDMEMBER"}, argv), out);
}

common::Error CommandsApi::Srem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SREM"}, argv), out);
}

common::Error CommandsApi::Sscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SSCAN"}, argv), out);
}

common::Error CommandsApi::StrLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"STRLEN"}, argv), out);
}

common::Error CommandsApi::Sunion(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SUNION"}, argv), out);
}

common::Error CommandsApi::SunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SUNIONSTORE"}, argv), out);
}

common::Error CommandsApi::Time(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"TIME"}, argv), out);
}

common::Error CommandsApi::Type(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"TYPE"}, argv), out);
}

common::Error CommandsApi::Unsubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"UNSUBSCRIBE"}, argv), out);
}

common::Error CommandsApi::Unwatch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"UNWATCH"}, argv), out);
}

common::Error CommandsApi::Wait(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"WAIT"}, argv), out);
}

common::Error CommandsApi::Watch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"WATCH"}, argv), out);
}

common::Error CommandsApi::Zcard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZCARD"}, argv), out);
}

common::Error CommandsApi::Zcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZCOUNT"}, argv), out);
}

common::Error CommandsApi::ZincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZINCRBY"}, argv), out);
}

common::Error CommandsApi::ZincrStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZINTERSTORE"}, argv), out);
}

common::Error CommandsApi::ZlexCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZLEXCOUNT"}, argv), out);
}

common::Error CommandsApi::ZrangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZRANGEBYLEX"}, argv), out);
}

common::Error CommandsApi::ZrangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZRANGEBYSCORE"}, argv), out);
}

common::Error CommandsApi::Zrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZRANK"}, argv), out);
}

common::Error CommandsApi::Zrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREM"}, argv), out);
}

common::Error CommandsApi::ZremRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREMRANGEBYLEX"}, argv), out);
}

common::Error CommandsApi::ZremRangeByRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREMRANGEBYRANK"}, argv), out);
}

common::Error CommandsApi::ZremRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREMRANGEBYSCORE"}, argv), out);
}

common::Error CommandsApi::ZrevRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREVRANGE"}, argv), out);
}

common::Error CommandsApi::ZrevRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREVRANGEBYLEX"}, argv), out);
}

common::Error CommandsApi::ZrevRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREVRANGEBYSCORE"}, argv), out);
}

common::Error CommandsApi::ZrevRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZREVRANK"}, argv), out);
}

common::Error CommandsApi::Zscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZSCAN"}, argv), out);
}

common::Error CommandsApi::Zscore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZSCORE"}, argv), out);
}

common::Error CommandsApi::ZunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ZUNIONSTORE"}, argv), out);
}

common::Error CommandsApi::SentinelMasters(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "MASTERS"}, argv), out);
}

common::Error CommandsApi::SentinelMaster(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "MASTER"}, argv), out);
}

common::Error CommandsApi::SentinelSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "SLAVES"}, argv), out);
}

common::Error CommandsApi::SentinelSentinels(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "SENTINELS"}, argv), out);
}

common::Error CommandsApi::SentinelGetMasterAddrByName(internal::CommandHandler* handler,
                                                       commands_args_t argv,
                                                       FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "GET-MASTER-ADDR-BY-NAME"}, argv), out);
}

common::Error CommandsApi::SentinelReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "RESET"}, argv), out);
}

common::Error CommandsApi::SentinelFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "FAILOVER"}, argv), out);
}

common::Error CommandsApi::SentinelCkquorum(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "CKQUORUM"}, argv), out);
}

common::Error CommandsApi::SentinelFlushConfig(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "FLUSHCONFIG"}, argv), out);
}

common::Error CommandsApi::SentinelMonitor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "MONITOR"}, argv), out);
}

common::Error CommandsApi::SentinelRemove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "REMOVE"}, argv), out);
}

common::Error CommandsApi::SentinelSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SENTINEL", "SET"}, argv), out);
}

common::Error CommandsApi::SetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  ttl_t ttl;
  if (!common::ConvertFromString(argv[1], &ttl)) {
    return common::make_error_inval();
  }
  NValue string_val(common::Value::CreateStringValue(argv[2]));
  NDbKValue kv(key, string_val);

  DBConnection* redis = static_cast<DBConnection*>(handler);
  common::Error err = redis->SetEx(kv, ttl);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::SetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  NValue string_val(common::Value::CreateStringValue(argv[1]));
  NDbKValue kv(key, string_val);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->SetNX(kv, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Sadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  common::SetValue* set = common::Value::CreateSetValue();
  for (size_t i = 1; i < argv.size(); ++i) {
    set->Insert(argv[i]);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long added_items = 0;
  common::Error err = redis->Sadd(key, NValue(set), &added_items);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(added_items);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Smembers(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Smembers(key, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  common::ZSetValue* zset = common::Value::CreateZSetValue();
  for (size_t i = 1; i < argv.size(); i += 2) {
    std::string key = argv[i];
    std::string val = argv[i + 1];
    zset->Insert(key, val);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long added_items = 0;
  common::Error err = redis->Zadd(key, NValue(zset), &added_items);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(added_items);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  int start;
  if (!common::ConvertFromString(argv[1], &start)) {
    return common::make_error_inval();
  }

  int stop;
  if (!common::ConvertFromString(argv[2], &stop)) {
    return common::make_error_inval();
  }
  bool ws = argv.size() == 4 && strncmp(argv[3].c_str(), "WITHSCORES", 10) == 0;
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Zrange(key, start, stop, ws, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hmset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  common::HashValue* hmset = common::Value::CreateHashValue();
  for (size_t i = 1; i < argv.size(); i += 2) {
    std::string key = argv[i];
    std::string val = argv[i + 1];
    hmset->Insert(key, val);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  common::Error err = redis->Hmset(key, NValue(hmset));
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hgetall(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Hgetall(key, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Decr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->Decr(key, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::DecrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  int incr;
  if (!common::ConvertFromString(argv[1], &incr)) {
    return common::make_error_inval();
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->DecrBy(key, incr, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->Incr(key, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::IncrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  int incr;
  if (!common::ConvertFromString(argv[1], &incr)) {
    return common::make_error_inval();
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->IncrBy(key, incr, &result);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::IncrByFloat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  double incr;
  if (!common::ConvertFromString(argv[1], &incr)) {
    return common::make_error_inval();
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  std::string result;
  common::Error err = redis->IncrByFloat(key, incr, &result);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(result);
  FastoObject* child = new FastoObject(out, val, redis->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Persist(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->SetTTL(key, NO_TTL);
  if (err) {
    common::FundamentalValue* val = common::Value::CreateUIntegerValue(0);
    FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
    out->AddChildren(child);
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(1);
  FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::ExpireRedis(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  ttl_t ttl;
  if (!common::ConvertFromString(argv[1], &ttl)) {
    return common::make_error_inval();
  }

  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->SetTTL(key, ttl);
  if (err) {
    common::FundamentalValue* val = common::Value::CreateUIntegerValue(0);
    FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
    out->AddChildren(child);
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(1);
  FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Monitor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MONITOR");
  return red->Monitor(argv, out);
}

common::Error CommandsApi::Subscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front(DB_SUBSCRIBE_COMMAND);
  return red->Subscribe(argv, out);
}

common::Error CommandsApi::Sync(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->SlaveMode(out);
}

// extend comands
common::Error CommandsApi::Latency(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"LATENCY"}, argv), out);
}

common::Error CommandsApi::PFDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PFDEBUG"}, argv), out);
}

common::Error CommandsApi::ReplConf(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"REPLCONF"}, argv), out);
}

common::Error CommandsApi::Substr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SUBSTR"}, argv), out);
}

common::Error CommandsApi::ModuleList(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MODULE", "LIST"}, argv), out);
}

common::Error CommandsApi::MemoryDoctor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MEMORY", "DOCTOR"}, argv), out);
}

common::Error CommandsApi::MemoryUsage(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MEMORY", "USAGE"}, argv), out);
}

common::Error CommandsApi::MemoryStats(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MEMORY", "STATS"}, argv), out);
}

common::Error CommandsApi::MemoryPurge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MEMORY", "PURGE"}, argv), out);
}

common::Error CommandsApi::MemoryMallocStats(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MEMORY", "MALLOC-STATS"}, argv), out);
}

common::Error CommandsApi::SwapDB(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"SWAPDB"}, argv), out);
}

common::Error CommandsApi::Unlink(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"UNLINK"}, argv), out);
}

common::Error CommandsApi::Touch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"TOUCH"}, argv), out);
}

common::Error CommandsApi::Xlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"XLEN"}, argv), out);
}

common::Error CommandsApi::Xrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"XRANGE"}, argv), out);
}

common::Error CommandsApi::Xread(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"XREAD"}, argv), out);
}

common::Error CommandsApi::Xadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"XADD"}, argv), out);
}

common::Error CommandsApi::PFSelfTest(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"PFSELFTEST"}, argv), out);
}

common::Error CommandsApi::Asking(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"ASKING"}, argv), out);
}

common::Error CommandsApi::RestoreAsking(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"RESTORE-ASKING"}, argv), out);
}

common::Error CommandsApi::GeoRadius_ro(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEORADIUS_RO"}, argv), out);
}

common::Error CommandsApi::GeoRadiusByMember_ro(internal::CommandHandler* handler,
                                                commands_args_t argv,
                                                FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"GEORADIUSBYMEMBER_RO"}, argv), out);
}

// modules
common::Error CommandsApi::GraphQuery(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphQuery(ExpandCommand({REDIS_GRAPH_MODULE_COMMAND("QUERY")}, argv), out);
}

common::Error CommandsApi::GraphExplain(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphExplain(ExpandCommand({REDIS_GRAPH_MODULE_COMMAND("EXPLAIN")}, argv), out);
}

common::Error CommandsApi::GraphDelete(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_GRAPH_MODULE_COMMAND("DELETE")}, argv), out);
}

common::Error CommandsApi::FtAdd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("ADD")}, argv), out);
}

common::Error CommandsApi::FtCreate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("CREATE")}, argv), out);
}

common::Error CommandsApi::FtSearch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SEARCH")}, argv), out);
}

common::Error CommandsApi::FtAddHash(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("ADDHASH")}, argv), out);
}

common::Error CommandsApi::FtInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("INFO")}, argv), out);
}

common::Error CommandsApi::FtOptimize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("OPTIMIZE")}, argv), out);
}

common::Error CommandsApi::FtExplain(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("EXPLAIN")}, argv), out);
}

common::Error CommandsApi::FtDel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("DEL")}, argv), out);
}

common::Error CommandsApi::FtGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("GET")}, argv), out);
}

common::Error CommandsApi::FtMGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("MGET")}, argv), out);
}

common::Error CommandsApi::FtDrop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("DROP")}, argv), out);
}

common::Error CommandsApi::FtSugadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SUGGADD")}, argv), out);
}

common::Error CommandsApi::FtSugget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SUGGET")}, argv), out);
}

common::Error CommandsApi::FtSugdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SUGDEL")}, argv), out);
}

common::Error CommandsApi::FtSuglen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SUGLEN")}, argv), out);
}

common::Error CommandsApi::JsonDel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("DEL")}, argv), out);
}

common::Error CommandsApi::JsonGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  DBConnection* red = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = red->JsonGet(key, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, red->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::JsonMget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("MGET")}, argv), out);
}

common::Error CommandsApi::JsonSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  NValue json_val(new JsonValue(common::ConvertToString(argv[2])));
  NDbKValue kv(key, json_val);

  DBConnection* red = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = red->JsonSet(kv, &key_added);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::JsonType(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("TYPE")}, argv), out);
}

common::Error CommandsApi::JsonNumIncrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("NUMINCRBY")}, argv), out);
}

common::Error CommandsApi::JsonNumMultBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("NUMMULTBY")}, argv), out);
}

common::Error CommandsApi::JsonStrAppend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("STRAPPEND")}, argv), out);
}

common::Error CommandsApi::JsonStrlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("STRLEN")}, argv), out);
}

common::Error CommandsApi::JsonArrAppend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRAPPEND")}, argv), out);
}

common::Error CommandsApi::JsonArrIndex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRINDEX")}, argv), out);
}

common::Error CommandsApi::JsonArrInsert(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRINSERT")}, argv), out);
}

common::Error CommandsApi::JsonArrLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRLEN")}, argv), out);
}

common::Error CommandsApi::JsonArrPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRPOP")}, argv), out);
}

common::Error CommandsApi::JsonArrTrim(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRTRIM")}, argv), out);
}

common::Error CommandsApi::JsonObjKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("OBJKEYS")}, argv), out);
}

common::Error CommandsApi::JsonObjLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("OBJLEN")}, argv), out);
}

common::Error CommandsApi::JsonDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("DEBUG")}, argv), out);
}

common::Error CommandsApi::JsonForget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("FORGET")}, argv), out);
}

common::Error CommandsApi::JsonResp(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_JSON_MODULE_COMMAND("RESP")}, argv), out);
}

common::Error CommandsApi::NrReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("RESET")}, argv), out);
}

common::Error CommandsApi::NrInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("INFO")}, argv), out);
}

common::Error CommandsApi::NrGetData(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("GETDATA")}, argv), out);
}

common::Error CommandsApi::NrRun(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("RUN")}, argv), out);
}

common::Error CommandsApi::NrClass(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("CLASS")}, argv), out);
}

common::Error CommandsApi::NrCreate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("CREATE")}, argv), out);
}

common::Error CommandsApi::NrObserve(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("OBSERVE")}, argv), out);
}

common::Error CommandsApi::NrTrain(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("TRAIN")}, argv), out);
}

common::Error CommandsApi::NrThreads(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({REDIS_NR_MODULE_COMMAND("THREADS")}, argv), out);
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
