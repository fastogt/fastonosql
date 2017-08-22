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

#include <string.h>  // for strncmp
#include <memory>    // for __shared_ptr

#include <common/convert2string.h>
#include <common/value.h>  // for Value, ErrorValue, etc

#include "core/db_key.h"

#include "core/db/redis/db_connection.h"

#include "core/global.h"

namespace fastonosql {
namespace core {
namespace redis {

common::Error CommandsApi::Auth(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->Auth(argv[0]);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  int stop;
  if (!common::ConvertFromString(argv[2], &stop)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Lrange(key, start, stop, &key_loaded);
  if (err && err->IsError()) {
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
  argv.push_front("INFO");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Append(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("APPEND");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BgRewriteAof(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BGREWRITEAOF");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BgSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BGSAVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BitCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BITCOUNT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BitField(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BITFIELD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BitOp(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BITOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BitPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BITPOS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BlPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BLPOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BrPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BRPOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::BrPopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("BRPOPLPUSH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientGetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("GETNAME");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("KILL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientList(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("LIST");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientPause(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("PAUSE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientReply(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("REPLY");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClientSetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLIENT");
  argv.push_front("SETNAME");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterAddSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("ADDSLOTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterCountFailureReports(internal::CommandHandler* handler,
                                                      commands_args_t argv,
                                                      FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("COUNT-FAILURE-REPORTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterCountKeysSinSlot(internal::CommandHandler* handler,
                                                   commands_args_t argv,
                                                   FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("COUNTKEYSINSLOT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterDelSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("DELSLOTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("FAILOVER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterForget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("FORGET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterGetKeySinSlot(internal::CommandHandler* handler,
                                                commands_args_t argv,
                                                FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("GETKEYSINSLOT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("INFO");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterKeySlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("KEYSLOT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterMeet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("MEET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterNodes(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("NODES");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterReplicate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("REPLICATE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("RESET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterSaveConfig(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("SAVECONFIG");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterSetConfigEpoch(internal::CommandHandler* handler,
                                                 commands_args_t argv,
                                                 FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("SET-CONFIG-EPOCH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterSetSlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("SETSLOT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("SLAVES");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ClusterSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CLUSTER");
  argv.push_front("SLOTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::CommandCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("COMMAND");
  argv.push_front("COUNT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::CommandGetKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("COMMAND");
  argv.push_front("GETKEYS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::CommandInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("COMMAND");
  argv.push_front("INFO");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Command(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("COMMAND");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ConfigGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CONFIG");
  argv.push_front("GET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ConfigResetStat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CONFIG");
  argv.push_front("RESETSTAT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ConfigRewrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CONFIG");
  argv.push_front("REWRITE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ConfigSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("CONFIG");
  argv.push_front("SET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::DbSize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("DBSIZE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::DebugObject(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("DEBUG");
  argv.push_front("OBJECT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::DebugSegFault(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("DEBUG");
  argv.push_front("SEGFAULT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Discard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("DISCARD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Dump(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("DUMP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Echo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ECHO");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Eval(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("EVAL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::EvalSha(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("EVALSHA");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Exec(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("EXEC");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Exists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("EXISTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ExpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("EXPIREAT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::FlushALL(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("FLUSHALL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoAdd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEOADD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoDist(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEODIST");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoHash(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEOHASH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEOPOS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoRadius(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEORADIUS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GeoRadiusByMember(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GEORADIUSBYMEMBER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GETBIT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GETRANGE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::GetSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("GETSET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HDEL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hexists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HEXISTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HGET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::HincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HINCRBY");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::HincrByFloat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HINCRBYFLOAT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HKEYS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HLEN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hmget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HMGET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HSCAN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HSET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::HsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HSETNX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hstrlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HSTRLEN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Hvals(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("HVALS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::RKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("KEYS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::LastSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LASTSAVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Lindex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LINDEX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Linsert(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LINSERT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Llen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LLEN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Lpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LPOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::LpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LPUSHX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Lrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LREM");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Lset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LSET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Ltrim(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("LTRIM");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MGET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Migrate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MIGRATE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Move(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MOVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Mset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MSET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::MsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MSETNX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Multi(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("MULTI");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Object(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("OBJECT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Pexpire(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PEXPIRE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::PexpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PEXPIREAT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Pfadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PFADD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Pfcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PFCOUNT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Pfmerge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PFMERGE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Ping(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PING");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::PsetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PSETEX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Pttl(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PTTL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Publish(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PUBLISH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::PubSub(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PUBSUB");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::PunSubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("PUNSUBSCRIBE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::RandomKey(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RANDOMKEY");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ReadOnly(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("READONLY");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ReadWrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("READWRITE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::RenameNx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RENAMENX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Restore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RESTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Role(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ROLE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Rpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RPOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::RpopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RPOPLPUSH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Rpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RPUSH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::RpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("RPUSHX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Save(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SAVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Scard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCARD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ScriptDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCRIPT");
  argv.push_front("DEBUG");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ScriptExists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCRIPT");
  argv.push_front("EXISTS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ScriptFlush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCRIPT");
  argv.push_front("FLUSH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ScriptKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCRIPT");
  argv.push_front("KILL");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ScriptLoad(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SCRIPT");
  argv.push_front("LOAD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Sdiff(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SDIFF");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SdiffStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SDIFFSTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SETBIT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SETRANGE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Shutdown(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SHUTDOWN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Sinter(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SINTER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SinterStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SINTERSTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SisMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SISMEMBER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SlaveOf(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SLAVEOF");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SlowLog(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SLOWLOG");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Smove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SMOVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Sort(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SORT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Spop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SPOP");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SRandMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SRANDMEMBER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Srem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SREM");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Sscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SSCAN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::StrLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("STRLEN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Sunion(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SUNION");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SUNIONSTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Time(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("TIME");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Type(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("TYPE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Unsubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("UNSUBSCRIBE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Unwatch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("UNWATCH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Wait(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("WAIT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Watch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("WATCH");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zcard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZCARD");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZCOUNT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZINCRBY");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZincrStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZINTERSTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZlexCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZLEXCOUNT");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZRANGEBYLEX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZRANGEBYSCORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZRANK");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREM");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZremRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREMRANGEBYLEX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZremRangeByRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREMRANGEBYRANK");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZremRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREMRANGEBYSCORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrevRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREVRANGE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrevRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREVRANGEBYLEX");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrevRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREVRANGEBYSCORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZrevRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZREVRANK");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZSCAN");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::Zscore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZSCORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::ZunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("ZUNIONSTORE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelMasters(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("MASTERS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelMaster(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("MASTER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("SLAVES");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelSentinels(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("SENTINELS");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelGetMasterAddrByName(internal::CommandHandler* handler,
                                                       commands_args_t argv,
                                                       FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("GET-MASTER-ADDR-BY-NAME");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("RESET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("FAILOVER");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelCkquorum(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("CKQUORUM");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelFlushConfig(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("FLUSHCONFIG");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelMonitor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("MONITOR");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelRemove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("REMOVE");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SentinelSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  argv.push_front("SENTINEL");
  argv.push_front("SET");
  return red->CommonExec(argv, out);
}

common::Error CommandsApi::SetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t key_str(argv[0]);
  NKey key(key_str);
  ttl_t ttl;
  if (!common::ConvertFromString(argv[1], &ttl)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }
  NValue string_val(common::Value::CreateStringValue(argv[2]));
  NDbKValue kv(key, string_val);

  DBConnection* redis = static_cast<DBConnection*>(handler);
  common::Error err = redis->SetEx(kv, ttl);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  int stop;
  if (!common::ConvertFromString(argv[2], &stop)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }
  bool ws = argv.size() == 4 && strncmp(argv[3].c_str(), "WITHSCORES", 10) == 0;
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Zrange(key, start, stop, ws, &key_loaded);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->DecrBy(key, incr, &result);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }
  DBConnection* redis = static_cast<DBConnection*>(handler);
  long long result = 0;
  common::Error err = redis->IncrBy(key, incr, &result);
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  std::string result;
  common::Error err = redis->IncrByFloat(key, incr, &result);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->SetTTL(key, ttl);
  if (err && err->IsError()) {
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
  argv.push_front("SUBSCRIBE");
  return red->Subscribe(argv, out);
}

common::Error CommandsApi::Sync(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->SlaveMode(out);
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
