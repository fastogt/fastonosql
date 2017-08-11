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

#include <common/error.h>   // for Error
#include <common/macros.h>  // for PROJECT_VERSION_GENERATE

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
namespace redis {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace core {
namespace redis {

struct CommandsApi : public internal::ApiTraits<DBConnection> {
  static common::Error Info(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Append(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BgRewriteAof(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BgSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BitCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BitField(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BitOp(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BitPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BlPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BrPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error BrPopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientGetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientList(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientPause(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientReply(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClientSetName(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterAddSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterCountFailureReports(internal::CommandHandler* handler,
                                                  commands_args_t argv,
                                                  FastoObject* out);
  static common::Error ClusterCountKeysSinSlot(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out);
  static common::Error ClusterDelSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterForget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterGetKeySinSlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterKeySlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterMeet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterNodes(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterReplicate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterSaveConfig(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterSetConfigEpoch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterSetSlot(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ClusterSlots(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error CommandCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error CommandGetKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error CommandInfo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Command(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ConfigGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ConfigResetStat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ConfigRewrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ConfigSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DbSize(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DebugObject(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DebugSegFault(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Discard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Dump(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Echo(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Eval(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error EvalSha(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Exec(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Exists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ExpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error FlushALL(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoAdd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoDist(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoHash(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoPos(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoRadius(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GeoRadiusByMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GetSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hdel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hexists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error HincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error HincrByFloat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hkeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hmget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error HsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hstrlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hvals(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error LastSave(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Lindex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Linsert(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Llen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Lpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error LpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Lrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Lset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Ltrim(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Mget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Migrate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Move(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Mset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error MsetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Multi(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Object(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Pexpire(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error PexpireAt(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Pfadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Pfcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Pfmerge(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Ping(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error PsetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Pttl(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Publish(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error PubSub(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error PunSubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RandomKey(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ReadOnly(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ReadWrite(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RenameNx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Restore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Role(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Rpop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RpopLpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Rpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RpushX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Save(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Scard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScriptDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScriptExists(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScriptFlush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScriptKill(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ScriptLoad(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sdiff(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SdiffStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SetBit(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SetRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Shutdown(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sinter(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SinterStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SisMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SlaveOf(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SlowLog(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Smove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sort(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Spop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SRandMember(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Srem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error StrLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sunion(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Time(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Type(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Unsubscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Unwatch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Wait(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Watch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zcard(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zcount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZincrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZincrStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZlexCount(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrem(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZremRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZremRangeByRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZremRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrevRange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrevRangeByLex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrevRangeByScore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZrevRank(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zscan(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zscore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ZunionStore(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelMasters(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelMaster(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelSlaves(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelSentinels(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelGetMasterAddrByName(internal::CommandHandler* handler,
                                                   commands_args_t argv,
                                                   FastoObject* out);
  static common::Error SentinelReset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelFailover(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelCkquorum(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelFlushConfig(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelMonitor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelRemove(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SentinelSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error SetEx(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SetNX(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Lpush(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Lrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Sadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Smembers(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Zadd(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Zrange(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Hmset(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Hgetall(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Decr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DecrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Incr(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error IncrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error IncrByFloat(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Persist(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ExpireRedis(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Auth(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Monitor(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Subscribe(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Sync(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
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
    CommandHolder("APPEND",
                  "<key> <value>",
                  "Append a value to a key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Append),
    CommandHolder("AUTH",
                  "<password>",
                  "Authenticate to the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Auth),
    CommandHolder("BGREWRITEAOF",
                  "-",
                  "Asynchronously rewrite the append-only file",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::BgRewriteAof),
    CommandHolder("BGSAVE",
                  "-",
                  "Asynchronously save the dataset to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::BgSave),
    CommandHolder("BITCOUNT",
                  "<key> [start] [end]",
                  "Count set bits in a string",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  2,
                  &CommandsApi::BitCount),
    CommandHolder("BITFIELD",
                  "key [GET type offset] [SET type offset value] [INCRBY type offset increment] "
                  "[OVERFLOW WRAP|SAT|FAIL]",
                  "Perform arbitrary bitfield integer operations on strings",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  8,
                  &CommandsApi::BitField),
    CommandHolder("BITOP",
                  "<operation> <destkey> <key> [key ...]",
                  "Perform bitwise operations between strings",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  2,
                  &CommandsApi::BitOp),
    CommandHolder("BITPOS",
                  "<key> <bit> [start] [end]",
                  "Find first bit set or clear in a string",
                  PROJECT_VERSION_GENERATE(2, 8, 7),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  2,
                  &CommandsApi::BitPos),
    CommandHolder("BLPOP",
                  "<key> [key ...] timeout",
                  "Remove and get the first element in a "
                  "list, or block until "
                  "one is available",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::BlPop),
    CommandHolder("BRPOP",
                  "<key> [key ...] timeout",
                  "Remove and get the last element in a "
                  "list, or block until "
                  "one is available",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::BrPop),
    CommandHolder("BRPOPLPUSH",
                  "source destination timeout",
                  "Pop a value from a list, push it to "
                  "another list and return "
                  "it; or block until "
                  "one is available",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::BrPopLpush),

    CommandHolder("CLIENT GETNAME",
                  "-",
                  "Get the current connection name",
                  PROJECT_VERSION_GENERATE(2, 6, 9),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClientGetName),
    CommandHolder("CLIENT KILL",
                  "[ip:port] [ID client-id] [TYPE "
                  "normal|master|slave|pubsub] "
                  "[ADDR ip:port] [SKIPME yes/no]",
                  "Kill the connection of a client",
                  PROJECT_VERSION_GENERATE(2, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  9,
                  &CommandsApi::ClientKill),
    CommandHolder("CLIENT LIST",
                  "-",
                  "Get the list of client connections",
                  PROJECT_VERSION_GENERATE(2, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClientList),
    CommandHolder("CLIENT PAUSE",
                  "<timeout>",
                  "Stop processing commands from clients "
                  "for some time",
                  PROJECT_VERSION_GENERATE(2, 9, 50),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClientPause),
    CommandHolder("CLIENT REPLY",
                  "ON|OFF|SKIP",
                  "Instruct the server whether to reply to commands",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClientReply),
    CommandHolder("CLIENT SETNAME",
                  "<connection-name>",
                  "Set the current connection name",
                  PROJECT_VERSION_GENERATE(2, 6, 9),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClientSetName),

    CommandHolder("CLUSTER ADDSLOTS",
                  "<slot> [slot ...]",
                  "Assign new hash slots to receiving node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::ClusterAddSlots),
    CommandHolder("CLUSTER COUNT-FAILURE-REPORTS",
                  "<node-id>",
                  "Return the number of failure reports "
                  "active for a given node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterCountFailureReports),
    CommandHolder("CLUSTER COUNTKEYSINSLOT",
                  "<slot>",
                  "Return the number of local keys in the "
                  "specified hash slot",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterCountKeysSinSlot),
    CommandHolder("CLUSTER DELSLOTS",
                  "<slot> [slot ...]",
                  "Set hash slots as unbound in receiving node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::ClusterDelSlots),
    CommandHolder("CLUSTER FAILOVER",
                  "[FORCE|TAKEOVER]",
                  "Forces a slave to perform a manual "
                  "failover osyncf its master.",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::ClusterFailover),
    CommandHolder("CLUSTER FORGET",
                  "<node-id>",
                  "Remove a node from the nodes table",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterForget),
    CommandHolder("CLUSTER GETKEYSINSLOT",
                  "<slot> <count>",
                  "Return local key names in the specified hash slot",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ClusterGetKeySinSlot),
    CommandHolder("CLUSTER INFO",
                  "-",
                  "Provides info about Redis Cluster node state",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClusterInfo),
    CommandHolder("CLUSTER KEYSLOT",
                  "<key>",
                  "Returns the hash slot of the specified key",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterKeySlot),
    CommandHolder("CLUSTER MEET",
                  "<ip> <port>",
                  "Force a node cluster to handshake with "
                  "another node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ClusterMeet),
    CommandHolder("CLUSTER NODES",
                  "-",
                  "Get Cluster config for the node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClusterNodes),
    CommandHolder("CLUSTER REPLICATE",
                  "<node-id>",
                  "Reconfigure a node as a slave of the "
                  "specified master node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterReplicate),
    CommandHolder("CLUSTER RESET",
                  "[HARD|SOFT]",
                  "Reset a Redis Cluster node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::ClusterReset),
    CommandHolder("CLUSTER SAVECONFIG",
                  "-",
                  "Forces the node to save cluster state on disk",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClusterSaveConfig),
    CommandHolder("CLUSTER SET-CONFIG-EPOCH",
                  "<config-epoch>",
                  "Set the configuration epoch in a new node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterSetConfigEpoch),
    CommandHolder("CLUSTER SETSLOT",
                  "<slot> IMPORTING|MIGRATING|STABLE|NODE [node-id]",
                  "Bind a hash slot to a specific node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  2,
                  &CommandsApi::ClusterSetSlot),
    CommandHolder("CLUSTER SLAVES",
                  "<node-id>",
                  "Licommon_execst slave nodes of the "
                  "specified master node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ClusterSlaves),
    CommandHolder("CLUSTER SLOTS",
                  "-",
                  "Get array of Cluster slot to node mappings",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ClusterSlots),

    CommandHolder("COMMAND COUNT",
                  "-",
                  "Get total number of Redis commands",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::CommandCount),
    CommandHolder("COMMAND GETKEYS",
                  "-",
                  "Extract keys given a full Redis command",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::CommandGetKeys),
    CommandHolder("COMMAND INFO",
                  "command-name [command-name ...]",
                  "Get array of specific Redis command details",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::CommandInfo),
    CommandHolder("COMMAND",
                  "-",
                  "Get array of Redis command details",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Command),

    CommandHolder("CONFIG GET",
                  "<parameter>",
                  "Get the value of a configuration parameter",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ConfigGet),
    CommandHolder("CONFIG RESETSTAT",
                  "-",
                  "Reset the stats returned by INFO",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ConfigResetStat),
    CommandHolder("CONFIG REWRITE",
                  "-",
                  "Rewrite the configuration file with the "
                  "in memory configuration",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ConfigRewrite),
    CommandHolder("CONFIG SET",
                  "<parameter> <value>",
                  "Set a configuration parameter to the given value",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ConfigSet),

    CommandHolder("DBSIZE",
                  "-",
                  "Return the number of keys in the "
                  "selected database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::DbSize),

    CommandHolder("DEBUG OBJECT",
                  "<key>",
                  "Get debugging information about a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::DebugObject),
    CommandHolder("DEBUG SEGFAULT",
                  "-",
                  "Make the server crash",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::DebugSegFault),

    CommandHolder("DECR",
                  "<key>",
                  "Decrement the integer value of a key by one",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Decr),
    CommandHolder("DECRBY",
                  "<key> <decrement>",
                  "Decrement the integer value of a key by "
                  "the given number",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::DecrBy),
    CommandHolder("DEL",
                  "<key> [key ...]",
                  "Delete a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Delete),
    CommandHolder("DISCARD",
                  "-",
                  "Discard all commands issued after MULTI",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Discard),
    CommandHolder("DUMP",
                  "<key>",
                  "Return a serialized version of the "
                  "value stored at the specified key.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Dump),
    CommandHolder("ECHO",
                  "<message>",
                  "Echo the given string",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Echo),
    CommandHolder("EVAL",
                  "script numkeys <key> [key ...] <arg> [arg ...]",
                  "Execute a Lua script server side",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Eval),
    CommandHolder("EVALSHA",
                  "sha1 numkeys <key> [key ...] <arg> [arg ...]",
                  "Execute a Lua script server side",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::EvalSha),
    CommandHolder("EXEC",
                  "-",
                  "Execute all commands issued after MULTI",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Exec),
    CommandHolder("EXISTS",
                  "key [key ...]",
                  "Determine if a key exists",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Exists),
    CommandHolder("EXPIRE",
                  "<key> <seconds>",
                  "Set a key's time to live in seconds",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ExpireRedis),
    CommandHolder("EXPIREAT",
                  "<key> <timestamp>",
                  "Set the expiration for a key as a UNIX timestamp",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ExpireAt),
    CommandHolder("FLUSHALL",
                  "-",
                  "Remove all keys from all databases",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::FlushALL),
    CommandHolder("FLUSHDB",
                  "-",
                  "Remove all keys from the current database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::FlushDB),
    CommandHolder("GEOADD",
                  "key longitude latitude member "
                  "[longitude latitude member ...",
                  "Add one or more geospatial items in the "
                  "geospatial index represented "
                  "using a sorted set",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &CommandsApi::GeoAdd),
    CommandHolder("GEODIST",
                  "key member1 member2 [unit]",
                  "Returns the distance between two "
                  "members of a geospatial index",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  &CommandsApi::GeoDist),
    CommandHolder("GEOHASH",
                  "key member [member ...]",
                  "Returns members of a geospatial index "
                  "as standard geohash strings",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::GeoHash),
    CommandHolder("GEOPOS",
                  "key member [member ...]",
                  "Returns longitude and latitude of "
                  "members of a geospatial index",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::GeoPos),

    CommandHolder("GEORADIUS",
                  "key longitude latitude radius "
                  "m|km|ft|mi [WITHCOORD] "
                  "[WITHDIST] [WITHHASH] "
                  "[COUNT count] [ASC|DESC]",
                  "Query a sorted set representing a "
                  "geospatial index to fetch "
                  "members matching a "
                  "given maximum distance from a point",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  5,
                  6,
                  &CommandsApi::GeoRadius),
    CommandHolder("GEORADIUSBYMEMBER",
                  "key member radius m|km|ft|mi "
                  "[WITHCOORD] [WITHDIST] "
                  "[WITHHASH] [COUNT count] [ASC|DESC]",
                  "Query a sorted set representing a "
                  "geospatial index to fetch "
                  "members matching a given "
                  "maximum distance from a member",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  6,
                  &CommandsApi::GeoRadiusByMember),
    CommandHolder("GET",
                  "<key>",
                  "Gecommon_exect the value of a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Get),
    CommandHolder("GETBIT",
                  "<key> <offset>",
                  "Returns the bit value at offset in the "
                  "string value stored at key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::GetBit),
    CommandHolder("GETRANGE",
                  "<key> <start> <end>",
                  "Get a substring of the string stored at a key",
                  PROJECT_VERSION_GENERATE(2, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::GetRange),
    CommandHolder("GETSET",
                  "<key> <value>",
                  "Set the string value of a key and "
                  "return its old value",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::GetSet),
    CommandHolder("HDEL",
                  "<key> <field> [field ...]",
                  "Delete one or more hash fields",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Hdel),
    CommandHolder("HEXISTS",
                  "<key> <field>",
                  "Determine if a hash field exists",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Hexists),
    CommandHolder("HGET",
                  "<key> <field>",
                  "Get the value of a hash field",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Hget),
    CommandHolder("HGETALL",
                  "<key>",
                  "Get all the fields and values in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Hgetall),
    CommandHolder("HINCRBY",
                  "<key> <field> <increment>",
                  "Increment the integer value of a hash "
                  "field by the given number",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::HincrByFloat),
    CommandHolder("HINCRBYFLOAT",
                  "<key> <field> <increment>",
                  "Increment the float value of a hash "
                  "field by the given amount",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::HincrByFloat),
    CommandHolder("HKEYS",
                  "<key>",
                  "Get all the fields in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Hkeys),
    CommandHolder("HLEN",
                  "<key>",
                  "Get the number of fields in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Hlen),
    CommandHolder("HMGET",
                  "<key> <field> [field ...]",
                  "Get the values of all the given hash fields",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Hmget),
    CommandHolder("HMSET",
                  "<key> <field> <value> [field value ...]",
                  "Set multiple hash fields to multiple values",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Hmset,
                  {&TestArgsInRange, &TestArgsModule2Equal1}),
    CommandHolder("HSCAN",
                  "<key> <cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate hash fields and associated "
                  "values",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  &CommandsApi::Hscan),
    CommandHolder("HSET",
                  "<key> <field> <value>",
                  "Set the string value of a hash field",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Hset),
    CommandHolder("HSETNX",
                  "<key> <field> <value>",
                  "Set the value of a hash field, only if "
                  "the field does not exist",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::HsetNX),
    CommandHolder("HSTRLEN",
                  "<key> <field>",
                  "Get the length of the value of a hash field",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Hstrlen),
    CommandHolder("HVALS",
                  "<key>",
                  "Get all the values in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Hvals),
    CommandHolder("INCR",
                  "<key>",
                  "Increment the integer value of a key by one",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Incr),
    CommandHolder("INCRBY",
                  "<key> <increment>",
                  "Increment the integer value of a key by "
                  "the given amount",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::IncrBy),
    CommandHolder("INCRBYFLOAT",
                  "<key> <increment>",
                  "Increment the float value of a key by "
                  "the given amount",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::IncrByFloat),
    CommandHolder("INFO",
                  "[section]",
                  "Get information and statistics about the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Info),
    CommandHolder("KEYS",
                  "<pattern>",
                  "Find all keys matching the given pattern",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::RKeys),
    CommandHolder("LASTSAVE",
                  "-",
                  "Get the UNIX time stamp of the last "
                  "successful save to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::LastSave),
    CommandHolder("LINDEX",
                  "<key> <index>",
                  "Get an element from a list by its index",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Lindex),
    CommandHolder("LINSERT",
                  "<key> <BEFORE|AFTER> <pivot value>",
                  "Insert an element before or after "
                  "another element in a list",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Linsert),
    CommandHolder("LLEN",
                  "<key>",
                  "Get the length of a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Llen),
    CommandHolder("LPOP",
                  "<key>",
                  "Remove and get the first element in a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Lpop),
    CommandHolder("LPUSH",
                  "<key> <value> [value ...]",
                  "Prepend one or multiple values to a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Lpush),
    CommandHolder("LPUSHX",
                  "<key> <value>",
                  "Prepend a value to a list, only if the "
                  "list exists",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::LpushX),
    CommandHolder("LRANGE",
                  "<key> <start> <stop>",
                  "Get a range of elements from a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Lrange),
    CommandHolder("LREM",
                  "<key> <count> <value>",
                  "Remove elements from a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Lrem),
    CommandHolder("LSET",
                  "<key> <index> <value>",
                  "Set the value of an element in a list "
                  "by its index",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Lset),
    CommandHolder("LTRIM",
                  "<key> <start> <stop>",
                  "Trim a list to the specified range",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Ltrim),
    CommandHolder("MGET",
                  "<key> [key ...]",
                  "Get the values of all the given keys",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Mget),
    CommandHolder("MIGRATE",
                  "<host> <port> <key> <destination-db> <timeout> [COPY] [REPLACE] [KEYS key]",
                  "Atomically transfer a key from a Redis instance "
                  "to another one.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  5,
                  2,
                  &CommandsApi::Migrate),
    CommandHolder("MONITOR",
                  "-",
                  "Listen for all requests received by the "
                  "server in real time",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Monitor),
    CommandHolder("MOVE",
                  "<key> <db>",
                  "Move a key to another database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Move),
    CommandHolder("MSET",
                  "<key> <value> [key value ...]",
                  "Set multiple keys to multiple values",
                  PROJECT_VERSION_GENERATE(1, 0, 1),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Mset),
    CommandHolder("MSETNX",
                  "<key> <value> [key value ...]",
                  "Set multiple keys to multiple values, "
                  "only if none of the keys exist",
                  PROJECT_VERSION_GENERATE(1, 0, 1),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::MsetNX),
    CommandHolder("MULTI",
                  "-",
                  "Mark the start of a transaction block",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Multi),
    CommandHolder("OBJECT",
                  "<subcommand> [arguments [arguments ...]]",
                  "Inspect the internals of Redis objects",
                  PROJECT_VERSION_GENERATE(2, 2, 3),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Object),
    CommandHolder("PERSIST",
                  "<key>",
                  "Remove the expiration from a key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Persist),
    CommandHolder("PEXPIRE",
                  "<key> <milliseconds>",
                  "Set a key's time to live in milliseconds",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Pexpire),
    CommandHolder("PEXPIREAT",
                  "<key> <milliseconds-timestamp>",
                  "Set the expiration for a key as a UNIX "
                  "timestamp specified "
                  "in milliseconds",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::PexpireAt),
    CommandHolder("PFADD",
                  "<key> <element> [element ...]",
                  "Adds the specified elements to the "
                  "specified HyperLogLog.",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Pfadd),
    CommandHolder("PFCOUNT",
                  "<key> [key ...]",
                  "Return the approximated cardinality of "
                  "the set(s) observed "
                  "by the HyperLogLog at key(s).",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Pfcount),
    CommandHolder("PFMERGE",
                  "<destkey> <sourcekey> [sourcekey ...]",
                  "Merge N different HyperLogLogs into a single one.",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Pfmerge),
    CommandHolder("PING",
                  "[message]",
                  "Ping the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Ping),
    CommandHolder("PSETEX",
                  "<key> <milliseconds> <value>",
                  "Set the value and expiration in "
                  "milliseconds of a key",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::PsetEx),
    CommandHolder("PSUBSCRIBE",
                  "<pattern> [pattern ...]",
                  "Listen for messages published to "
                  "channels matching the given patterns",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Subscribe),
    CommandHolder("PTTL",
                  "<key>",
                  "Get the time to live for a key in milliseconds",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Pttl),
    CommandHolder("PUBLISH",
                  "<channel> <message>",
                  "Post a message to a channel",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Publish),
    CommandHolder("PUBSUB",
                  "<subcommand> [argument [argument ...]]",
                  "Inspect the state of the Pub/Sub subsystem",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::PubSub),
    CommandHolder("PUNSUBSCRIBE",
                  "[pattern [pattern ...]]",
                  "Stop listening for messages posted to "
                  "channels matching the "
                  "given patterns",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::PunSubscribe),
    CommandHolder("QUIT",
                  "-",
                  "Close the connection",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Quit),
    CommandHolder("RANDOMKEY",
                  "-",
                  "Return a random key from the keyspace",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::RandomKey),
    CommandHolder("READONLY",
                  "-",
                  "Enables read queries for a connection "
                  "to a cluster slave node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ReadOnly),
    CommandHolder("READWRITE",
                  "-",
                  "Disables read queries for a connection "
                  "to a cluster slave node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ReadWrite),
    CommandHolder("RENAME",
                  "<key> <newkey>",
                  "Rename a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Rename),
    CommandHolder("RENAMENX",
                  "<key> <newkey>",
                  "Rename a key, only if the new key does not exist",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::RenameNx),
    CommandHolder("RESTORE",
                  "<key> <ttl> <serialized-value> [REPLACE]",
                  "Create a key using the provided serialized value, "
                  "previously obtained using DUMP.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  &CommandsApi::Restore),
    CommandHolder("ROLE",
                  "-",
                  "Return the role of the instance in the "
                  "context of replication",
                  PROJECT_VERSION_GENERATE(2, 8, 12),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Role),
    CommandHolder("RPOP",
                  "<key>",
                  "Remove and get the last element in a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Rpop),
    CommandHolder("RPOPLPUSH",
                  "<source> <destination>",
                  "Remove the last element in a list, prepend it to another list and return it",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::RpopLpush),
    CommandHolder("RPUSH",
                  "<key> <value> [value ...]",
                  "Append one or multiple values to a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Rpush),
    CommandHolder("RPUSHX",
                  "<key> <value>",
                  "Append a value to a list, only if the list exists",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::RpushX),
    CommandHolder("SADD",
                  "<key> <member> [member ...]",
                  "Add one or more members to a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Sadd),
    CommandHolder("SAVE",
                  "-",
                  "Synchronously save the dataset to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Save),
    CommandHolder("SCAN",
                  "<cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate the keys space",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  4,
                  &CommandsApi::Scan),
    CommandHolder("SCARD",
                  "<key>",
                  "Get the number of members in a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Scard),

    CommandHolder("SCRIPT DEBUG",
                  "<YES|SYNC|NO>",
                  "Set the debug mode for executed scripts.",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ScriptDebug),
    CommandHolder("SCRIPT EXISTS",
                  "script [script ...]",
                  "Check existence of scripts in the script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::ScriptExists),
    CommandHolder("SCRIPT FLUSH",
                  "-",
                  "Remove all the scripts from the script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ScriptFlush),
    CommandHolder("SCRIPT KILL",
                  "-",
                  "Kill the script currently in execution.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::ScriptKill),
    CommandHolder("SCRIPT LOAD",
                  "<script>",
                  "Load the specified Lua script into the "
                  "script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::ScriptLoad),

    CommandHolder("SDIFF",
                  "<key> [key ...]",
                  "Subtract multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Sdiff),
    CommandHolder("SDIFFSTORE",
                  "<destination> <key> [key ...]",
                  "Subtract multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::SdiffStore),
    CommandHolder("SELECT",
                  "<index>",
                  "Change the selected database for the "
                  "current connection",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Select),
    CommandHolder("SET",
                  "<key> <value> [EX seconds] [PX milliseconds] [NX|XX]",
                  "Set the string value of a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  5,
                  &CommandsApi::Set),
    CommandHolder("SETBIT",
                  "<key> <offset> <value>",
                  "Sets or clears the bit at offset in the "
                  "string value stored at key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::SetBit),
    CommandHolder("SETEX",
                  "<key> <seconds> <value>",
                  "Set the value and expiration of a key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::SetEx),
    CommandHolder("SETNX",
                  "<key> <value>",
                  "Set the value of a key, only if the key "
                  "does not exist",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::SetNX),
    CommandHolder("SETRANGE",
                  "<key> <offset> <value>",
                  "Overwrite part of a string at key "
                  "starting at the specified offset",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::SetRange),
    CommandHolder("SHUTDOWN",
                  "[NOSAVE|SAVE]",
                  "Synchronously save the dataset to disk "
                  "and then shut down the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  &CommandsApi::Shutdown),
    CommandHolder("SINTER",
                  "<key> [key ...]",
                  "Intersect multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Sinter),
    CommandHolder("SINTERSTORE",
                  "<destination> <key> [key ...]",
                  "Intersect multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::SinterStore),
    CommandHolder("SISMEMBER",
                  "<key> <member>",
                  "Determine if a given value is a member of a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::SisMember),
    CommandHolder("SLAVEOF",
                  "<host> <port>",
                  "Make the server a slave of another "
                  "instance, or promote it as master",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::SlaveOf),
    CommandHolder("SLOWLOG",
                  "<subcommand> [argument]",
                  "Manages the Redis slow queries log",
                  PROJECT_VERSION_GENERATE(2, 2, 12),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  &CommandsApi::SlowLog),
    CommandHolder("SMEMBERS",
                  "<key>",
                  "Get all the members in a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Smembers),
    CommandHolder("SMOVE",
                  "<source> <destination> <member>",
                  "Move a member from one set to another",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Smove),
    CommandHolder("SORT",
                  "<key> [BY pattern] [LIMIT offset count] [GET "
                  "pattern [GET "
                  "pattern ...]] "
                  "[ASC|DESC] [ALPHA] [STORE destination]",
                  "Sort the elements in a list, set or sorted set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Sort),
    CommandHolder("SPOP",
                  "<key> [count]",
                  "Remove and return one or multiple random members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  &CommandsApi::Spop),
    CommandHolder("SRANDMEMBER",
                  "<key> [count]",
                  "Get one or multiple random members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  &CommandsApi::SRandMember),
    CommandHolder("SREM",
                  "<key> <member> [member ...]",
                  "Remove one or more members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Srem),
    CommandHolder("SSCAN",
                  "<key> <cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate Set elements",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  &CommandsApi::Sscan),
    CommandHolder("STRLEN",
                  "<key>",
                  "Get the length of the value stored in a key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::StrLen),
    CommandHolder("SUBSCRIBE",
                  "<channel> [channel ...]",
                  "Listen for messages published to the "
                  "given channels",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Subscribe),
    CommandHolder("SUNION",
                  "<key> [key ...]",
                  "Add multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Sunion),
    CommandHolder("SUNIONSTORE",
                  "<destination> <key> [key ...]",
                  "Add multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::SunionStore),
    CommandHolder("SYNC",
                  "-",
                  "Internal command used for replication",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Sync),
    CommandHolder("PSYNC",
                  "-",
                  "Internal command used for replication",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Sync),
    CommandHolder("TIME",
                  "-",
                  "Return the current server time",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Time),
    CommandHolder("TTL",
                  "<key>",
                  "Get the time to live for a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::GetTTL),
    CommandHolder("TYPE",
                  "<key>",
                  "Determine the type stored at key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Type),
    CommandHolder("UNSUBSCRIBE",
                  "[channel [channel ...]]",
                  "Stop listening for messages posted to "
                  "the given channels",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Unsubscribe),
    CommandHolder("UNWATCH",
                  "-",
                  "Forget about all watched keys",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::Unwatch),
    CommandHolder("WAIT",
                  "<numslaves> <timeout>",
                  "Wait for the synchronous replication of "
                  "all the write "
                  "commands sent in the "
                  "context of the current connection",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Wait),
    CommandHolder("WATCH",
                  "key [key ...]",
                  "Watch the given keys to determine "
                  "execution of the MULTI/EXEC block",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Watch),
    CommandHolder("ZADD",
                  "<key> [NX|XX] [CH] [INCR] <score> "
                  "<member> [score member ...]",
                  "Add one or more members to a sorted "
                  "set, or update its score if it "
                  "already exists",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Zadd),
    CommandHolder("ZCARD",
                  "<key>",
                  "Get the number of members in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::Zcard),
    CommandHolder("ZCOUNT",
                  "<key> <min> <max>",
                  "Count the members in a sorted set with "
                  "scores within the given values",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::Zcount),
    CommandHolder("ZINCRBY",
                  "<key> <increment> <member>",
                  "Increment the score of a member in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ZincrBy),
    CommandHolder("ZINTERSTORE",
                  "<destination> <numkeys> <key> [key ...] "
                  "[WEIGHTS weight] "
                  "[AGGREGATE SUM|MIN|MAX]",
                  "Intersect multiple sorted sets and "
                  "store the resulting "
                  "sorted set in a new key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::ZincrStore),
    CommandHolder("ZLEXCOUNT",
                  "<key> <min> <max>",
                  "Count the number of members in a sorted "
                  "set between a given "
                  "lexicographical range",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ZlexCount),
    CommandHolder("ZRANGE",
                  "<key> <start> <stop> [WITHSCORES]",
                  "Return a range of members in a sorted "
                  "set, by index",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  &CommandsApi::Zrange),
    CommandHolder("ZRANGEBYLEX",
                  "<key> <min> <max> [LIMIT offset count]",
                  "Return a range of members in a sorted "
                  "set, by lexicographical range",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  3,
                  &CommandsApi::ZrangeByLex),
    CommandHolder("ZRANGEBYSCORE",
                  "<key> <min> <max> [WITHSCORES] [LIMIT "
                  "offset count]",
                  "Return a range of members in a sorted "
                  "set, by score",
                  PROJECT_VERSION_GENERATE(1, 0, 5),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  4,
                  &CommandsApi::ZrangeByScore),
    CommandHolder("ZRANK",
                  "<key> <member>",
                  "Determine the index of a member in a sorted set",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Zrank),
    CommandHolder("ZREM",
                  "<key> <member> [member ...]",
                  "Remove one or more members from a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::Zrem),
    CommandHolder("ZREMRANGEBYLEX",
                  "<key> <min> <max>",
                  "Remove all members in a sorted set "
                  "between the given "
                  "lexicographical range",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ZremRangeByLex),
    CommandHolder("ZREMRANGEBYRANK",
                  "<key> <start> <stop>",
                  "Remove all members in a sorted set "
                  "within the given indexes",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ZremRangeByRank),
    CommandHolder("ZREMRANGEBYSCORE",
                  "<key> <min> <max>",
                  "Remove all members in a sorted set "
                  "within the given scores",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::ZremRangeByScore),
    CommandHolder("ZREVRANGE",
                  "<key> <start> <stop> [WITHSCORES]",
                  "Return a range of members in a sorted "
                  "set, by index, with "
                  "scores ordered from high to low",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  &CommandsApi::ZrevRange),
    CommandHolder("ZREVRANGEBYLEX",
                  "<key> <max> <min> [LIMIT offset count]",
                  "Return a range of members in a sorted set, by "
                  "lexicographical range, ordered "
                  "from higher to lower strings.",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  3,
                  &CommandsApi::ZrevRangeByLex),
    CommandHolder("ZREVRANGEBYSCORE",
                  "<key> <max> <min> [WITHSCORES] [LIMIT "
                  "offset count]",
                  "Return a range of members in a sorted "
                  "set, by score, with "
                  "scores ordered from high to low",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  4,
                  &CommandsApi::ZrevRangeByScore),
    CommandHolder("ZREVRANK",
                  "<key> <member>",
                  "Determine the index of a member in a "
                  "sorted set, with "
                  "scores ordered from high to low",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::ZrevRank),
    CommandHolder("ZSCAN",
                  "<key> <cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate sorted sets elements and "
                  "associated scores",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  &CommandsApi::Zscan),
    CommandHolder("ZSCORE",
                  "<key> <member>",
                  "Get the score associated with the given "
                  "member in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  &CommandsApi::Zscore),
    CommandHolder("ZUNIONSTORE",
                  "<destination> <numkeys> <key> [key ...] "
                  "[WEIGHTS weight] "
                  "[AGGREGATE SUM|MIN|MAX]",
                  "Add multiple sorted sets and store the "
                  "resulting sorted set "
                  "in a new key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  &CommandsApi::ZunionStore),

    CommandHolder("SENTINEL MASTERS",
                  "-",
                  "Show a list of monitored masters and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::SentinelMasters),
    CommandHolder("SENTINEL MASTER",
                  "<master name>",
                  "Show the state and info of the specified master.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelMaster),
    CommandHolder("SENTINEL SLAVES",
                  "<master name>",
                  "Show a list of slaves for this master, "
                  "and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelSlaves),
    CommandHolder("SENTINEL SENTINELS",
                  "<master name>",
                  "Show a list of sentinel instances for "
                  "this master, and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelSentinels),
    CommandHolder("SENTINEL GET-MASTER-ADDR-BY-NAME",
                  "<master name>",
                  "Return the ip and port number of the "
                  "master with that name.\n"
                  "If a failover is in progress or "
                  "terminated successfully for this "
                  "master "
                  "it returns the address and port of the "
                  "promoted slave.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelGetMasterAddrByName),
    CommandHolder("SENTINEL RESET",
                  "<pattern>",
                  "This command will reset all the masters with "
                  "matching name.\n"
                  "The pattern argument is a glob-style pattern.\n"
                  "The reset process clears any previous state in a "
                  "master (including a "
                  "failover in "
                  "progress), "
                  "and removes every slave and sentinel already "
                  "discovered and "
                  "associated with the master.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelReset),
    CommandHolder("SENTINEL FAILOVER",
                  "<master name>",
                  "Force a failover as if the master was not "
                  "reachable, "
                  "and without asking for agreement to other "
                  "Sentinels "
                  "(however a new version of the configuration will "
                  "be "
                  "published so that the other "
                  "Sentinels will update their configurations).",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelFailover),
    CommandHolder("SENTINEL CKQUORUM",
                  "<master name>",
                  "Check if the current Sentinel "
                  "configuration is able to "
                  "reach the quorum needed "
                  "to failover a master, "
                  "and the majority needed to authorize "
                  "the failover.\n"
                  "This command should be used in "
                  "monitoring systems to check "
                  "if a Sentinel "
                  "deployment is ok.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelCkquorum),
    CommandHolder("SENTINEL FLUSHCONFIG",
                  "-",
                  "Force Sentinel to rewrite its "
                  "configuration on disk, including the "
                  "current Sentinel "
                  "state.\n"
                  "Normally Sentinel rewrites the "
                  "configuration every time something "
                  "changes in its state "
                  "(in the context of the subset of the "
                  "state which is persisted on disk "
                  "across restart).\n"
                  "However sometimes it is possible that "
                  "the configuration file is lost "
                  "because of operation "
                  "errors, "
                  "disk failures, package upgrade scripts "
                  "or configuration managers.\n"
                  "In those cases a way to to force "
                  "Sentinel to rewrite the "
                  "configuration file is handy.\n"
                  "This command works even if the previous "
                  "configuration file is "
                  "completely missing.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  &CommandsApi::SentinelFlushConfig),
    CommandHolder("SENTINEL MONITOR",
                  "<name> <ip> <port> <quorum>",
                  "This command tells the Sentinel to "
                  "start monitoring a new master with "
                  "the "
                  "specified name, ip, port, and quorum.\n"
                  "It is identical to the sentinel monitor "
                  "configuration directive in "
                  "sentinel.conf configuration file, "
                  "with the difference that you can't use "
                  "an hostname in as ip, but you "
                  "need to "
                  "provide an IPv4 or IPv6 address.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  &CommandsApi::SentinelMonitor),
    CommandHolder("SENTINEL REMOVE",
                  "<name>",
                  "Used in order to remove the specified "
                  "master: the master will no "
                  "longer be monitored, "
                  "and will totally be removed from the "
                  "internal state of the Sentinel, "
                  "so it will no longer listed by SENTINEL "
                  "masters and so forth.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  &CommandsApi::SentinelRemove),
    CommandHolder("SENTINEL SET",
                  "<name> <option> <value>",
                  "The SET command is very similar to the "
                  "CONFIG SET command of Redis, "
                  "and is used in order to change "
                  "configuration parameters of a specific "
                  "master.\n"
                  "Multiple option / value pairs can be "
                  "specified (or none at all).\n"
                  "All the configuration parameters that "
                  "can be configured via "
                  "sentinel.conf are also configurable "
                  "using the SET command.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  &CommandsApi::SentinelSet)};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
