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

#include "core/db/redis/db_connection.h"

#include <errno.h>

extern "C" {
#include "sds.h"
}

#include <hiredis/hiredis.h>
#include <libssh2.h>  // for libssh2_exit, etc

#include <common/convert2string.h>

#include "core/db/redis/cluster_infos.h"  // for makeDiscoveryClusterInfo
#include "core/db/redis/command_translator.h"
#include "core/db/redis/database_info.h"  // for DataBaseInfo
#include "core/db/redis/internal/commands_api.h"
#include "core/db/redis/internal/modules.h"
#include "core/db/redis/sentinel_info.h"  // for DiscoverySentinelInfo, etc
#include "core/value.h"

#define HIREDIS_VERSION    \
  STRINGIZE(HIREDIS_MAJOR) \
  "." STRINGIZE(HIREDIS_MINOR) "." STRINGIZE(HIREDIS_PATCH)

#define DBSIZE "DBSIZE"

#define GET_PASSWORD "CONFIG get requirepass"

#define LATENCY_SAMPLE_RATE 10                 /* milliseconds. */
#define LATENCY_HISTORY_DEFAULT_INTERVAL 15000 /* milliseconds. */

#define RTYPE_STRING 0
#define RTYPE_LIST 1
#define RTYPE_SET 2
#define RTYPE_HASH 3
#define RTYPE_ZSET 4
#define RTYPE_NONE 5

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

namespace {

const struct RedisInit {
  RedisInit() { libssh2_init(0); }
  ~RedisInit() { libssh2_exit(); }
} rInit;

bool isPipeLineCommand(const char* command) {
  if (!command) {
    DNOTREACHED();
    return false;
  }

  bool skip =
      strcasecmp(command, "quit") == 0 || strcasecmp(command, "exit") == 0 || strcasecmp(command, "connect") == 0 ||
      strcasecmp(command, "help") == 0 || strcasecmp(command, "?") == 0 || strcasecmp(command, "shutdown") == 0 ||
      strcasecmp(command, "monitor") == 0 || strcasecmp(command, "subscribe") == 0 ||
      strcasecmp(command, "psubscribe") == 0 || strcasecmp(command, "sync") == 0 || strcasecmp(command, "psync") == 0;

  return !skip;
}

}  // namespace

namespace fastonosql {
namespace core {
namespace redis {
namespace {
const ConstantCommandsArray g_commands = {
    CommandHolder(DB_HELP_COMMAND,
                  "[command]",
                  "Return how to use command",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Help),
    CommandHolder("APPEND",
                  "<key> <value>",
                  "Append a value to a key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Append),
    CommandHolder("AUTH",
                  "<password>",
                  "Authenticate to the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Auth),
    CommandHolder("BGREWRITEAOF",
                  "-",
                  "Asynchronously rewrite the append-only file",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::BgRewriteAof),
    CommandHolder("BGSAVE",
                  "-",
                  "Asynchronously save the dataset to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::BgSave),
    CommandHolder("BITCOUNT",
                  "<key> [start] [end]",
                  "Count set bits in a string",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  2,
                  CommandInfo::Native,
                  &CommandsApi::BitCount),
    CommandHolder("BITFIELD",
                  "key [GET type offset] [SET type offset value] [INCRBY type offset increment] "
                  "[OVERFLOW WRAP|SAT|FAIL]",
                  "Perform arbitrary bitfield integer operations on strings",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  8,
                  CommandInfo::Native,
                  &CommandsApi::BitField),
    CommandHolder("BITOP",
                  "<operation> <destkey> <key> [key ...]",
                  "Perform bitwise operations between strings",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  2,
                  CommandInfo::Native,
                  &CommandsApi::BitOp),
    CommandHolder("BITPOS",
                  "<key> <bit> [start] [end]",
                  "Find first bit set or clear in a string",
                  PROJECT_VERSION_GENERATE(2, 8, 7),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  2,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::BrPopLpush),

    CommandHolder("CLIENT GETNAME",
                  "-",
                  "Get the current connection name",
                  PROJECT_VERSION_GENERATE(2, 6, 9),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ClientKill),
    CommandHolder("CLIENT LIST",
                  "-",
                  "Get the list of client connections",
                  PROJECT_VERSION_GENERATE(2, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClientList),
    CommandHolder("CLIENT PAUSE",
                  "<timeout>",
                  "Stop processing commands from clients "
                  "for some time",
                  PROJECT_VERSION_GENERATE(2, 9, 50),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClientPause),
    CommandHolder("CLIENT REPLY",
                  "ON|OFF|SKIP",
                  "Instruct the server whether to reply to commands",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClientReply),
    CommandHolder("CLIENT SETNAME",
                  "<connection-name>",
                  "Set the current connection name",
                  PROJECT_VERSION_GENERATE(2, 6, 9),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClientSetName),

    CommandHolder("CLUSTER ADDSLOTS",
                  "<slot> [slot ...]",
                  "Assign new hash slots to receiving node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::ClusterAddSlots),
    CommandHolder("CLUSTER COUNT-FAILURE-REPORTS",
                  "<node-id>",
                  "Return the number of failure reports "
                  "active for a given node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterCountFailureReports),
    CommandHolder("CLUSTER COUNTKEYSINSLOT",
                  "<slot>",
                  "Return the number of local keys in the "
                  "specified hash slot",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterCountKeysSinSlot),
    CommandHolder("CLUSTER DELSLOTS",
                  "<slot> [slot ...]",
                  "Set hash slots as unbound in receiving node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::ClusterDelSlots),
    CommandHolder("CLUSTER FAILOVER",
                  "[FORCE|TAKEOVER]",
                  "Forces a slave to perform a manual "
                  "failover osyncf its master.",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::ClusterFailover),
    CommandHolder("CLUSTER FORGET",
                  "<node-id>",
                  "Remove a node from the nodes table",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterForget),
    CommandHolder("CLUSTER GETKEYSINSLOT",
                  "<slot> <count>",
                  "Return local key names in the specified hash slot",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterGetKeySinSlot),
    CommandHolder("CLUSTER INFO",
                  "-",
                  "Provides info about Redis Cluster node state",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterInfo),
    CommandHolder("CLUSTER KEYSLOT",
                  "<key>",
                  "Returns the hash slot of the specified key",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterKeySlot),
    CommandHolder("CLUSTER MEET",
                  "<ip> <port>",
                  "Force a node cluster to handshake with "
                  "another node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterMeet),
    CommandHolder("CLUSTER NODES",
                  "-",
                  "Get Cluster config for the node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterNodes),
    CommandHolder("CLUSTER REPLICATE",
                  "<node-id>",
                  "Reconfigure a node as a slave of the "
                  "specified master node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterReplicate),
    CommandHolder("CLUSTER RESET",
                  "[HARD|SOFT]",
                  "Reset a Redis Cluster node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::ClusterReset),
    CommandHolder("CLUSTER SAVECONFIG",
                  "-",
                  "Forces the node to save cluster state on disk",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterSaveConfig),
    CommandHolder("CLUSTER SET-CONFIG-EPOCH",
                  "<config-epoch>",
                  "Set the configuration epoch in a new node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterSetConfigEpoch),
    CommandHolder("CLUSTER SETSLOT",
                  "<slot> IMPORTING|MIGRATING|STABLE|NODE [node-id]",
                  "Bind a hash slot to a specific node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  2,
                  CommandInfo::Native,
                  &CommandsApi::ClusterSetSlot),
    CommandHolder("CLUSTER SLAVES",
                  "<node-id>",
                  "Licommon_execst slave nodes of the "
                  "specified master node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterSlaves),
    CommandHolder("CLUSTER SLOTS",
                  "-",
                  "Get array of Cluster slot to node mappings",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ClusterSlots),

    CommandHolder("COMMAND COUNT",
                  "-",
                  "Get total number of Redis commands",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::CommandCount),
    CommandHolder("COMMAND GETKEYS",
                  "-",
                  "Extract keys given a full Redis command",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::CommandGetKeys),
    CommandHolder("COMMAND INFO",
                  "command-name [command-name ...]",
                  "Get array of specific Redis command details",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::CommandInfo),
    CommandHolder("COMMAND",
                  "-",
                  "Get array of Redis command details",
                  PROJECT_VERSION_GENERATE(2, 8, 13),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Command),

    CommandHolder("CONFIG GET",
                  "<parameter>",
                  "Get the value of a configuration parameter",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ConfigGet),
    CommandHolder("CONFIG RESETSTAT",
                  "-",
                  "Reset the stats returned by INFO",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ConfigResetStat),
    CommandHolder("CONFIG REWRITE",
                  "-",
                  "Rewrite the configuration file with the "
                  "in memory configuration",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ConfigRewrite),
    CommandHolder("CONFIG SET",
                  "<parameter> <value>",
                  "Set a configuration parameter to the given value",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ConfigSet),

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
    CommandHolder("DBSIZE",
                  "-",
                  "Return the number of keys in the "
                  "selected database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::DbSize),

    CommandHolder("DEBUG OBJECT",
                  "<key>",
                  "Get debugging information about a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::DebugObject),
    CommandHolder("DEBUG SEGFAULT",
                  "-",
                  "Make the server crash",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::DebugSegFault),

    CommandHolder("DECR",
                  "<key>",
                  "Decrement the integer value of a key by one",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Decr),
    CommandHolder("DECRBY",
                  "<key> <decrement>",
                  "Decrement the integer value of a key by "
                  "the given number",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::DecrBy),
    CommandHolder(DB_DELETE_KEY_COMMAND,
                  "<key> [key ...]",
                  "Delete a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Delete),
    CommandHolder("DISCARD",
                  "-",
                  "Discard all commands issued after MULTI",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Discard),
    CommandHolder("DUMP",
                  "<key>",
                  "Return a serialized version of the "
                  "value stored at the specified key.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Dump),
    CommandHolder("ECHO",
                  "<message>",
                  "Echo the given string",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Echo),
    CommandHolder("EVAL",
                  "script numkeys <key> [key ...] <arg> [arg ...]",
                  "Execute a Lua script server side",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Eval),
    CommandHolder("EVALSHA",
                  "sha1 numkeys <key> [key ...] <arg> [arg ...]",
                  "Execute a Lua script server side",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::EvalSha),
    CommandHolder("EXEC",
                  "-",
                  "Execute all commands issued after MULTI",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Exec),
    CommandHolder("EXISTS",
                  "key [key ...]",
                  "Determine if a key exists",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Exists),
    CommandHolder(DB_SET_TTL_COMMAND,
                  "<key> <seconds>",
                  "Set a key's time to live in seconds",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ExpireRedis),
    CommandHolder("EXPIREAT",
                  "<key> <timestamp>",
                  "Set the expiration for a key as a UNIX timestamp",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ExpireAt),
    CommandHolder("FLUSHALL",
                  "-",
                  "Remove all keys from all databases",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::FlushALL),
    CommandHolder(DB_FLUSHDB_COMMAND,
                  "-",
                  "Remove all keys from the current database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::GeoAdd),
    CommandHolder("GEODIST",
                  "key member1 member2 [unit]",
                  "Returns the distance between two "
                  "members of a geospatial index",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::GeoDist),
    CommandHolder("GEOHASH",
                  "key member [member ...]",
                  "Returns members of a geospatial index "
                  "as standard geohash strings",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::GeoHash),
    CommandHolder("GEOPOS",
                  "key member [member ...]",
                  "Returns longitude and latitude of "
                  "members of a geospatial index",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::GeoRadiusByMember),
    CommandHolder(DB_GET_KEY_COMMAND,
                  "<key>",
                  "Gecommon_exect the value of a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Get),
    CommandHolder("GETBIT",
                  "<key> <offset>",
                  "Returns the bit value at offset in the "
                  "string value stored at key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::GetBit),
    CommandHolder("GETRANGE",
                  "<key> <start> <end>",
                  "Get a substring of the string stored at a key",
                  PROJECT_VERSION_GENERATE(2, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::GetRange),
    CommandHolder("GETSET",
                  "<key> <value>",
                  "Set the string value of a key and "
                  "return its old value",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::GetSet),
    CommandHolder("HDEL",
                  "<key> <field> [field ...]",
                  "Delete one or more hash fields",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Hdel),
    CommandHolder("HEXISTS",
                  "<key> <field>",
                  "Determine if a hash field exists",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hexists),
    CommandHolder("HGET",
                  "<key> <field>",
                  "Get the value of a hash field",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hget),
    CommandHolder("HGETALL",
                  "<key>",
                  "Get all the fields and values in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hgetall),
    CommandHolder("HINCRBY",
                  "<key> <field> <increment>",
                  "Increment the integer value of a hash "
                  "field by the given number",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::HincrByFloat),
    CommandHolder("HINCRBYFLOAT",
                  "<key> <field> <increment>",
                  "Increment the float value of a hash "
                  "field by the given amount",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::HincrByFloat),
    CommandHolder("HKEYS",
                  "<key>",
                  "Get all the fields in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hkeys),
    CommandHolder("HLEN",
                  "<key>",
                  "Get the number of fields in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hlen),
    CommandHolder("HMGET",
                  "<key> <field> [field ...]",
                  "Get the values of all the given hash fields",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Hmget),
    CommandHolder("HMSET",
                  "<key> <field> <value> [field value ...]",
                  "Set multiple hash fields to multiple values",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::Hscan),
    CommandHolder("HSET",
                  "<key> <field> <value>",
                  "Set the string value of a hash field",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hset),
    CommandHolder("HSETNX",
                  "<key> <field> <value>",
                  "Set the value of a hash field, only if "
                  "the field does not exist",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::HsetNX),
    CommandHolder("HSTRLEN",
                  "<key> <field>",
                  "Get the length of the value of a hash field",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hstrlen),
    CommandHolder("HVALS",
                  "<key>",
                  "Get all the values in a hash",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Hvals),
    CommandHolder("INCR",
                  "<key>",
                  "Increment the integer value of a key by one",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Incr),
    CommandHolder("INCRBY",
                  "<key> <increment>",
                  "Increment the integer value of a key by "
                  "the given amount",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::IncrBy),
    CommandHolder("INCRBYFLOAT",
                  "<key> <increment>",
                  "Increment the float value of a key by "
                  "the given amount",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::IncrByFloat),
    CommandHolder(DB_INFO_COMMAND,
                  "[section]",
                  "Get information and statistics about the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Info),
    CommandHolder(DB_KEYS_COMMAND,
                  "<pattern>",
                  "Find all keys matching the given pattern",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::RKeys),
    CommandHolder("LASTSAVE",
                  "-",
                  "Get the UNIX time stamp of the last "
                  "successful save to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::LastSave),
    CommandHolder("LINDEX",
                  "<key> <index>",
                  "Get an element from a list by its index",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Lindex),
    CommandHolder("LINSERT",
                  "<key> <BEFORE|AFTER> <pivot value>",
                  "Insert an element before or after "
                  "another element in a list",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Linsert),
    CommandHolder("LLEN",
                  "<key>",
                  "Get the length of a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Llen),
    CommandHolder("LPOP",
                  "<key>",
                  "Remove and get the first element in a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Lpop),
    CommandHolder("LPUSH",
                  "<key> <value> [value ...]",
                  "Prepend one or multiple values to a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Lpush),
    CommandHolder("LPUSHX",
                  "<key> <value>",
                  "Prepend a value to a list, only if the "
                  "list exists",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::LpushX),
    CommandHolder("LRANGE",
                  "<key> <start> <stop>",
                  "Get a range of elements from a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Lrange),
    CommandHolder("LREM",
                  "<key> <count> <value>",
                  "Remove elements from a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Lrem),
    CommandHolder("LSET",
                  "<key> <index> <value>",
                  "Set the value of an element in a list "
                  "by its index",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Lset),
    CommandHolder("LTRIM",
                  "<key> <start> <stop>",
                  "Trim a list to the specified range",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Ltrim),
    CommandHolder("MGET",
                  "<key> [key ...]",
                  "Get the values of all the given keys",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Mget),
    CommandHolder("MIGRATE",
                  "<host> <port> <key> <destination-db> <timeout> [COPY] [REPLACE] [KEYS key]",
                  "Atomically transfer a key from a Redis instance "
                  "to another one.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  5,
                  2,
                  CommandInfo::Native,
                  &CommandsApi::Migrate),
    CommandHolder("MONITOR",
                  "-",
                  "Listen for all requests received by the "
                  "server in real time",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Monitor),
    CommandHolder("MOVE",
                  "<key> <db>",
                  "Move a key to another database",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Move),
    CommandHolder("MSET",
                  "<key> <value> [key value ...]",
                  "Set multiple keys to multiple values",
                  PROJECT_VERSION_GENERATE(1, 0, 1),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Mset),
    CommandHolder("MSETNX",
                  "<key> <value> [key value ...]",
                  "Set multiple keys to multiple values, "
                  "only if none of the keys exist",
                  PROJECT_VERSION_GENERATE(1, 0, 1),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::MsetNX),
    CommandHolder("MULTI",
                  "-",
                  "Mark the start of a transaction block",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Multi),
    CommandHolder("OBJECT",
                  "<subcommand> [arguments [arguments ...]]",
                  "Inspect the internals of Redis objects",
                  PROJECT_VERSION_GENERATE(2, 2, 3),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Object),
    CommandHolder("PERSIST",
                  "<key>",
                  "Remove the expiration from a key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Persist),
    CommandHolder("PEXPIRE",
                  "<key> <milliseconds>",
                  "Set a key's time to live in milliseconds",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::PexpireAt),
    CommandHolder("PFADD",
                  "<key> <element> [element ...]",
                  "Adds the specified elements to the "
                  "specified HyperLogLog.",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::Pfcount),
    CommandHolder("PFMERGE",
                  "<destkey> <sourcekey> [sourcekey ...]",
                  "Merge N different HyperLogLogs into a single one.",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Pfmerge),
    CommandHolder("PING",
                  "[message]",
                  "Ping the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Ping),
    CommandHolder("PSETEX",
                  "<key> <milliseconds> <value>",
                  "Set the value and expiration in "
                  "milliseconds of a key",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::PsetEx),
    CommandHolder("PSUBSCRIBE",
                  "<pattern> [pattern ...]",
                  "Listen for messages published to "
                  "channels matching the given patterns",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Subscribe),
    CommandHolder("PTTL",
                  "<key>",
                  "Get the time to live for a key in milliseconds",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Pttl),
    CommandHolder(DB_PUBLISH_COMMAND,
                  "<channel> <message>",
                  "Post a message to a channel",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Publish),
    CommandHolder("PUBSUB",
                  "<subcommand> [argument [argument ...]]",
                  "Inspect the state of the Pub/Sub subsystem",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::PunSubscribe),
    CommandHolder(DB_QUIT_COMMAND,
                  "-",
                  "Close the connection",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Quit),
    CommandHolder("RANDOMKEY",
                  "-",
                  "Return a random key from the keyspace",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::RandomKey),
    CommandHolder("READONLY",
                  "-",
                  "Enables read queries for a connection "
                  "to a cluster slave node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ReadOnly),
    CommandHolder("READWRITE",
                  "-",
                  "Disables read queries for a connection "
                  "to a cluster slave node",
                  PROJECT_VERSION_GENERATE(3, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ReadWrite),
    CommandHolder(DB_RENAME_KEY_COMMAND,
                  "<key> <newkey>",
                  "Rename a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Rename),
    CommandHolder("RENAMENX",
                  "<key> <newkey>",
                  "Rename a key, only if the new key does not exist",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::RenameNx),
    CommandHolder("RESTORE",
                  "<key> <ttl> <serialized-value> [REPLACE]",
                  "Create a key using the provided serialized value, "
                  "previously obtained using DUMP.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Restore),
    CommandHolder("ROLE",
                  "-",
                  "Return the role of the instance in the "
                  "context of replication",
                  PROJECT_VERSION_GENERATE(2, 8, 12),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Role),
    CommandHolder("RPOP",
                  "<key>",
                  "Remove and get the last element in a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Rpop),
    CommandHolder("RPOPLPUSH",
                  "<source> <destination>",
                  "Remove the last element in a list, prepend it to another list and return it",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::RpopLpush),
    CommandHolder("RPUSH",
                  "<key> <value> [value ...]",
                  "Append one or multiple values to a list",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Rpush),
    CommandHolder("RPUSHX",
                  "<key> <value>",
                  "Append a value to a list, only if the list exists",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::RpushX),
    CommandHolder("SADD",
                  "<key> <member> [member ...]",
                  "Add one or more members to a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Sadd),
    CommandHolder("SAVE",
                  "-",
                  "Synchronously save the dataset to disk",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Save),
    CommandHolder(DB_SCAN_COMMAND,
                  "<cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate the keys space",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  4,
                  CommandInfo::Native,
                  &CommandsApi::Scan),
    CommandHolder("SCARD",
                  "<key>",
                  "Get the number of members in a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Scard),

    CommandHolder("SCRIPT DEBUG",
                  "<YES|SYNC|NO>",
                  "Set the debug mode for executed scripts.",
                  PROJECT_VERSION_GENERATE(3, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ScriptDebug),
    CommandHolder("SCRIPT EXISTS",
                  "script [script ...]",
                  "Check existence of scripts in the script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::ScriptExists),
    CommandHolder("SCRIPT FLUSH",
                  "-",
                  "Remove all the scripts from the script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ScriptFlush),
    CommandHolder("SCRIPT KILL",
                  "-",
                  "Kill the script currently in execution.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ScriptKill),
    CommandHolder("SCRIPT LOAD",
                  "<script>",
                  "Load the specified Lua script into the "
                  "script cache.",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ScriptLoad),

    CommandHolder("SDIFF",
                  "<key> [key ...]",
                  "Subtract multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Sdiff),
    CommandHolder("SDIFFSTORE",
                  "<destination> <key> [key ...]",
                  "Subtract multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::SdiffStore),
    CommandHolder(DB_SELECTDB_COMMAND,
                  "<index>",
                  "Change the selected database for the "
                  "current connection",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Select),
    CommandHolder(DB_SET_KEY_COMMAND,
                  "<key> <value> [EX seconds] [PX milliseconds] [NX|XX]",
                  "Set the string value of a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  5,
                  CommandInfo::Native,
                  &CommandsApi::Set),
    CommandHolder("SETBIT",
                  "<key> <offset> <value>",
                  "Sets or clears the bit at offset in the "
                  "string value stored at key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SetBit),
    CommandHolder("SETEX",
                  "<key> <seconds> <value>",
                  "Set the value and expiration of a key",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SetEx),
    CommandHolder("SETNX",
                  "<key> <value>",
                  "Set the value of a key, only if the key "
                  "does not exist",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SetNX),
    CommandHolder("SETRANGE",
                  "<key> <offset> <value>",
                  "Overwrite part of a string at key "
                  "starting at the specified offset",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SetRange),
    CommandHolder("SHUTDOWN",
                  "[NOSAVE|SAVE]",
                  "Synchronously save the dataset to disk "
                  "and then shut down the server",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Shutdown),
    CommandHolder("SINTER",
                  "<key> [key ...]",
                  "Intersect multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Sinter),
    CommandHolder("SINTERSTORE",
                  "<destination> <key> [key ...]",
                  "Intersect multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::SinterStore),
    CommandHolder("SISMEMBER",
                  "<key> <member>",
                  "Determine if a given value is a member of a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SisMember),
    CommandHolder("SLAVEOF",
                  "<host> <port>",
                  "Make the server a slave of another "
                  "instance, or promote it as master",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SlaveOf),
    CommandHolder("SLOWLOG",
                  "<subcommand> [argument]",
                  "Manages the Redis slow queries log",
                  PROJECT_VERSION_GENERATE(2, 2, 12),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::SlowLog),
    CommandHolder("SMEMBERS",
                  "<key>",
                  "Get all the members in a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Smembers),
    CommandHolder("SMOVE",
                  "<source> <destination> <member>",
                  "Move a member from one set to another",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::Sort),
    CommandHolder("SPOP",
                  "<key> [count]",
                  "Remove and return one or multiple random members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Spop),
    CommandHolder("SRANDMEMBER",
                  "<key> [count]",
                  "Get one or multiple random members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::SRandMember),
    CommandHolder("SREM",
                  "<key> <member> [member ...]",
                  "Remove one or more members from a set",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Srem),
    CommandHolder("SSCAN",
                  "<key> <cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate Set elements",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  CommandInfo::Native,
                  &CommandsApi::Sscan),
    CommandHolder("STRLEN",
                  "<key>",
                  "Get the length of the value stored in a key",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::StrLen),
    CommandHolder(DB_SUBSCRIBE_COMMAND,
                  "<channel> [channel ...]",
                  "Listen for messages published to the "
                  "given channels",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Subscribe),
    CommandHolder("SUNION",
                  "<key> [key ...]",
                  "Add multiple sets",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Sunion),
    CommandHolder("SUNIONSTORE",
                  "<destination> <key> [key ...]",
                  "Add multiple sets and store the "
                  "resulting set in a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::SunionStore),
    CommandHolder("SYNC",
                  "-",
                  "Internal command used for replication",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Sync),
    CommandHolder("PSYNC",
                  "-",
                  "Internal command used for replication",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Sync),
    CommandHolder("TIME",
                  "-",
                  "Return the current server time",
                  PROJECT_VERSION_GENERATE(2, 6, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Time),
    CommandHolder(DB_GET_TTL_COMMAND,
                  "<key>",
                  "Get the time to live for a key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::GetTTL),
    CommandHolder("TYPE",
                  "<key>",
                  "Determine the type stored at key",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Type),
    CommandHolder("UNSUBSCRIBE",
                  "[channel [channel ...]]",
                  "Stop listening for messages posted to "
                  "the given channels",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
                  &CommandsApi::Unsubscribe),
    CommandHolder("UNWATCH",
                  "-",
                  "Forget about all watched keys",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::Wait),
    CommandHolder("WATCH",
                  "key [key ...]",
                  "Watch the given keys to determine "
                  "execution of the MULTI/EXEC block",
                  PROJECT_VERSION_GENERATE(2, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::Zadd),
    CommandHolder("ZCARD",
                  "<key>",
                  "Get the number of members in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Zcard),
    CommandHolder("ZCOUNT",
                  "<key> <min> <max>",
                  "Count the members in a sorted set with "
                  "scores within the given values",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Zcount),
    CommandHolder("ZINCRBY",
                  "<key> <increment> <member>",
                  "Increment the score of a member in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ZlexCount),
    CommandHolder("ZRANGE",
                  "<key> <start> <stop> [WITHSCORES]",
                  "Return a range of members in a sorted "
                  "set, by index",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  1,
                  CommandInfo::Native,
                  &CommandsApi::Zrange),
    CommandHolder("ZRANGEBYLEX",
                  "<key> <min> <max> [LIMIT offset count]",
                  "Return a range of members in a sorted "
                  "set, by lexicographical range",
                  PROJECT_VERSION_GENERATE(2, 8, 9),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  3,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ZrangeByScore),
    CommandHolder("ZRANK",
                  "<key> <member>",
                  "Determine the index of a member in a sorted set",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::Zrank),
    CommandHolder("ZREM",
                  "<key> <member> [member ...]",
                  "Remove one or more members from a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ZremRangeByLex),
    CommandHolder("ZREMRANGEBYRANK",
                  "<key> <start> <stop>",
                  "Remove all members in a sorted set "
                  "within the given indexes",
                  PROJECT_VERSION_GENERATE(2, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::ZremRangeByRank),
    CommandHolder("ZREMRANGEBYSCORE",
                  "<key> <min> <max>",
                  "Remove all members in a sorted set "
                  "within the given scores",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ZrevRank),
    CommandHolder("ZSCAN",
                  "<key> <cursor> [MATCH pattern] [COUNT count]",
                  "Incrementally iterate sorted sets elements and "
                  "associated scores",
                  PROJECT_VERSION_GENERATE(2, 8, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  CommandInfo::Native,
                  &CommandsApi::Zscan),
    CommandHolder("ZSCORE",
                  "<key> <member>",
                  "Get the score associated with the given "
                  "member in a sorted set",
                  PROJECT_VERSION_GENERATE(1, 2, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::ZunionStore),

    CommandHolder("SENTINEL MASTERS",
                  "-",
                  "Show a list of monitored masters and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SentinelMasters),
    CommandHolder("SENTINEL MASTER",
                  "<master name>",
                  "Show the state and info of the specified master.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SentinelMaster),
    CommandHolder("SENTINEL SLAVES",
                  "<master name>",
                  "Show a list of slaves for this master, "
                  "and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
                  &CommandsApi::SentinelSlaves),
    CommandHolder("SENTINEL SENTINELS",
                  "<master name>",
                  "Show a list of sentinel instances for "
                  "this master, and their state.",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::SentinelReset),
    CommandHolder("SENTINEL FAILOVER",
                  "<master name>",
                  "Force a failover as if the master was not "
                  "reachable, "
                  "and without asking for agreement to other "
                  "GetSentinels "
                  "(however a new version of the configuration will "
                  "be "
                  "published so that the other "
                  "GetSentinels will update their configurations).",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
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
                  CommandInfo::Native,
                  &CommandsApi::SentinelSet),
    // extended
    CommandHolder("LATENCY",
                  "<arg> <arg>  [options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Latency),
    CommandHolder("PFDEBUG",
                  "<arg> <arg> <arg> [options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::PFDebug),
    CommandHolder("REPLCONF",
                  "<arg> [options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::ReplConf),
    CommandHolder("SUBSTR",
                  "<key> <arg> <arg> <arg>",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::Substr),
    CommandHolder("PFSELFTEST",
                  "<arg>",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::PFSelfTest),
    CommandHolder("MODULE LIST",
                  "[options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  "MODULE LIST",
                  0,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::ModuleList),
    CommandHolder("MODULE LOAD",
                  "<module_path> [options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  "MODULE LOAD /home/sasha/Downloads/redis-graph/src/redisgraph.so",
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::ModuleLoad),
    CommandHolder("MODULE UNLOAD",
                  "<module_name> [options ...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  "MODULE UNLOAD graph",
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::ModuleUnLoad),
    CommandHolder("MEMORY DOCTOR",
                  "-",
                  "Outputs memory problems report",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::MemoryDoctor),
    CommandHolder("MEMORY USAGE",
                  "-",
                  "Estimate memory usage of key",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::MemoryUsage),
    CommandHolder("MEMORY STATS",
                  "-",
                  "Show memory usage details",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::MemoryStats),
    CommandHolder("MEMORY PURGE",
                  "-",
                  "Ask the allocator to release memory",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::MemoryPurge),
    CommandHolder("MEMORY MALLOC-STATS",
                  "-",
                  "Show allocator internal stats",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  0,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::MemoryMallocStats),
    CommandHolder("SWAPDB",
                  "<db1> <db2> [arg]",
                  "Swap db",
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::SwapDB),
    CommandHolder("UNLINK",
                  "<key> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Unlink),
    CommandHolder("TOUCH",
                  "<key> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Touch),
    CommandHolder("XLEN",
                  "<key> <arg>",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::Xlen),
    CommandHolder("XRANGE",
                  "<key> <arg> <arg> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Xrange),
    CommandHolder("XREAD",
                  "<key> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Xread),
    CommandHolder("XADD",
                  "<key> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Xadd),
    CommandHolder("ASKING",
                  "<key> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::Asking),
    CommandHolder("RESTORE-ASKING",
                  "<key> <arg> <arg> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::RestoreAsking),

    CommandHolder("GEORADIUS_RO",
                  "<key> <arg> <arg> <arg> <arg> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  6,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::GeoRadius_ro),
    CommandHolder("GEORADIUSBYMEMBER_RO",
                  "<key> <arg> <arg> <arg> <arg> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  5,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::GeoRadiusByMember_ro),

    // redis-graph api
    CommandHolder(REDIS_GRAPH_MODULE_COMMAND("QUERY"),
                  "<Graph name> <Query>",
                  "Executes the given query against a specified graph.",
                  PROJECT_VERSION_GENERATE(4, 0, 0),
                  REDIS_GRAPH_MODULE_COMMAND(
                      "QUERY") " us_government \"MATCH (p:president)-[:born]->(:state {name:Hawaii}) RETURN p\"",
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::GraphQuery),
    CommandHolder(REDIS_GRAPH_MODULE_COMMAND("EXPLAIN"),
                  "<Graph name> <Query>",
                  "Constructs a query execution plan but does not run it. Inspect this execution plan to better "
                  "understand how your query will get executed.",
                  PROJECT_VERSION_GENERATE(4, 0, 0),
                  REDIS_GRAPH_MODULE_COMMAND(
                      "EXPLAIN") " us_government \"MATCH (p:president)-[:born]->(h:state {name:Hawaii}) RETURN p\"",
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::GraphExplain),
    CommandHolder(REDIS_GRAPH_MODULE_COMMAND("DELETE"),
                  "<Graph name>",
                  "Delete graph by name",
                  PROJECT_VERSION_GENERATE(4, 0, 0),
                  REDIS_GRAPH_MODULE_COMMAND("DELETE") " us_government",
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::GraphDelete),
    // redisearch
    CommandHolder(
        REDIS_SEARCH_MODULE_COMMAND("CREATE"),
        "<index_name> [field weight ...]",
        "Creates an index with the given spec. The index name will be used in all the key names so keep it short!",
        PROJECT_VERSION_GENERATE(3, 4, 0),
        REDIS_SEARCH_MODULE_COMMAND("CREATE") " docs title 2.0 body 1.0 url 1.5",
        3,
        INFINITE_COMMAND_ARGS,
        CommandInfo::Extended,
        &CommandsApi::FtCreate),
    CommandHolder(
        REDIS_SEARCH_MODULE_COMMAND("ADD"),
        "<index_name> <doc_id> <score> [NOSAVE] FIELDS [field content ...]",
        "Add a documet to the index.",
        PROJECT_VERSION_GENERATE(3, 4, 0),
        REDIS_SEARCH_MODULE_COMMAND(
            "ADD") " docs doc1 1.0 FIELDS title “war and peace” body \"Well, Prince, so Genoa and Lucca are now…\"",
        6,
        INFINITE_COMMAND_ARGS,
        CommandInfo::Extended,
        &CommandsApi::FtAdd),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("ADDHASH"),
                  "<index> <docId> <score> [LANGUAGE language] [REPLACE]",
                  "Add a documet to the index.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  3,
                  CommandInfo::Extended,
                  &CommandsApi::FtAddHash),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("INFO"),
                  "<index>",
                  "Return information and statistics on the index.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  REDIS_SEARCH_MODULE_COMMAND("INFO") " wik{0}",
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtInfo),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SEARCH"),
                  "<index> <query> [NOCONTENT] [LIMIT offset num]",
                  "Search the index with a textual query, returning either documents or just ids.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  REDIS_SEARCH_MODULE_COMMAND("SEARCH") " idx \"hello world\" LIMIT 0 1",
                  5,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::FtSearch),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("EXPLAIN"),
                  "<index> <query>",
                  "Return the execution plan for a complex query.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtExplain),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("DEL"),
                  "<index> <doc_id>",
                  "Delete a document from the index.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtDel),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("GET"),
                  "<index> <doc_id>",
                  "Returns the full contents of a document.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtGet),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("MGET"),
                  "<index> <doc_id>",
                  "Returns the full contents of multiple documents.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtMGet),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("DROP"),
                  "<index>",
                  "Deletes all the keys associated with the index.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtDrop),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SUGADD"),
                  "<key> <string> <score> [INCR] [PAYLOAD payload]",
                  "Add a suggestion string to an auto-complete suggestion dictionary.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  3,
                  CommandInfo::Extended,
                  &CommandsApi::FtSugadd),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SUGGET"),
                  "<key> <prefix> [FUZZY] [WITHPAYLOADS] [MAX num]",
                  "Get completion suggestions for a prefix.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  4,
                  CommandInfo::Extended,
                  &CommandsApi::FtSugget),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SUGDEL"),
                  "<key> <string>",
                  "Delete a string from a suggestion index.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtSugdel),
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SUGLEN"),
                  "<key>",
                  "Get the size of an autoc-complete suggestion dictionary.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtSuglen),

    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("OPTIMIZE"),
                  "<index>",
                  "This command is deprecated. Index optimizations are done by the internal garbage collector in the "
                  "background. Client libraries should not implement this command, and remove it if they haven't "
                  "already.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  REDIS_SEARCH_MODULE_COMMAND("SEARCH") " wik{0}",
                  1,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::FtOptimize),

    // json
    CommandHolder(REDIS_JSON_MODULE_COMMAND("DEL"),
                  "<key> <path>",
                  "Delete a value.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::JsonDel),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("GET"),
                  "<key> [INDENT indentation-string][NEWLINE line-break-string][SPACE space-string] [path...]",
                  "Return the value at path in JSON serialized form.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  REDIS_SEARCH_MODULE_COMMAND("GET") " myjsonkey INDENT \"\t\" NEWLINE \"\n\" SPACE "
                                                     " path.to.value[1]",
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::JsonGet),
    CommandHolder(
        REDIS_JSON_MODULE_COMMAND("MGET"),
        "<key> [key ...] <path>",
        "Returns the values at path from multiple keys. Non-existing keys and non-existing paths are reported as null.",
        PROJECT_VERSION_GENERATE(1, 0, 0),
        UNDEFINED_EXAMPLE_STR,
        2,
        INFINITE_COMMAND_ARGS,
        CommandInfo::Extended,
        &CommandsApi::JsonMget),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("SET"),
                  "<key> <path> <json> [NX | XX]",
                  "Sets the JSON value at path in key.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  2,
                  CommandInfo::Extended,
                  &CommandsApi::JsonSet),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("TYPE"),
                  "<key> [path]",
                  "Report the type of JSON value at path.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonType),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("NUMINCRBY"),
                  "<key> <path> <number>",
                  "Increments the number value stored at path by number.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::JsonNumIncrBy),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("NUMMULTBY"),
                  "<key> <path> <number>",
                  "Multiplies the number value stored at path by number.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::JsonNumMultBy),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("STRAPPEND"),
                  "<key> [path] <json-string>",
                  "Append the json-string value(s) the string at path.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonStrAppend),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("STRLEN"),
                  "<key> [path]",
                  "Report the length of the JSON String at path in key.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonStrlen),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRAPPEND"),
                  "<key> <path> <json> [json ...]",
                  "Append the json value(s) into the array at path after the last element in it.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrAppend),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRINDEX"),
                  "<key> <path> <json-scalar> [start [stop]]",
                  "Search for the first occurrence of a scalar JSON value in an array.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  3,
                  2,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrIndex),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRINSERT"),
                  "<key> <path> <index> <json> [json ...]",
                  "Insert the json value(s) into the array at path before the index (shifts to the right).",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrInsert),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRLEN"),
                  "<key> [path]",
                  "Report the length of the JSON Array at path in key.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrLen),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRPOP"),
                  "<key> [path [index]]",
                  "Remove and return element from the index in the array.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  2,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrPop),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("ARRTRIM"),
                  "<key> <path> <start> <stop>",
                  "Trim an array so that it contains only the specified inclusive range of elements.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  4,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::JsonArrTrim),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("OBJKEYS"),
                  "<key> [path]",
                  "Return the keys in the object that's referenced by path.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonObjKeys),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("OBJLEN"),
                  "<key> [path]",
                  "Report the number of keys in the JSON Object at path in key.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonObjLen),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("DEBUG"),
                  "<subcommand & arguments>",
                  "Report information.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::JsonObjLen),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("FORGET"),
                  "<key> <path>",
                  "Delete a value.",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  2,
                  0,
                  CommandInfo::Extended,
                  &CommandsApi::JsonForget),
    CommandHolder(REDIS_JSON_MODULE_COMMAND("RESP"),
                  "<key> [path]",
                  "Return the JSON in key in Redis Serialization Protocol (RESP).",
                  PROJECT_VERSION_GENERATE(1, 0, 0),
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::JsonResp),
    // nr
    CommandHolder(REDIS_NR_MODULE_COMMAND("RESET"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrReset),
    CommandHolder(REDIS_NR_MODULE_COMMAND("INFO"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrInfo),
    CommandHolder(REDIS_NR_MODULE_COMMAND("GETDATA"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrGetData),
    CommandHolder(REDIS_NR_MODULE_COMMAND("RUN"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrRun),
    CommandHolder(REDIS_NR_MODULE_COMMAND("CLASS"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrClass),
    CommandHolder(REDIS_NR_MODULE_COMMAND("CREATE"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrCreate),
    CommandHolder(REDIS_NR_MODULE_COMMAND("OBSERVE"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrObserve),
    CommandHolder(REDIS_NR_MODULE_COMMAND("TRAIN"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrTrain),
    CommandHolder(REDIS_NR_MODULE_COMMAND("THREADS"),
                  "<key> [options...]",
                  UNDEFINED_SUMMARY,
                  UNDEFINED_SINCE,
                  UNDEFINED_EXAMPLE_STR,
                  1,
                  INFINITE_COMMAND_ARGS,
                  CommandInfo::Extended,
                  &CommandsApi::NrThreads)};

const ConstantCommandsArray g_internal_commands = {CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SETPAYLOAD"),
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SAFEADD"),
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SAFEADDHASH"),
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder(REDIS_SEARCH_MODULE_COMMAND("DTADD"),
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder(REDIS_SEARCH_MODULE_COMMAND("TERMADD"),
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder("post",
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr),
                                                   CommandHolder("host:",
                                                                 UNDEFINED_ARGS,
                                                                 UNDEFINED_SUMMARY,
                                                                 UNDEFINED_SINCE,
                                                                 UNDEFINED_EXAMPLE_STR,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 INFINITE_COMMAND_ARGS,
                                                                 CommandInfo::Internal,
                                                                 nullptr)};

}  // namespace
}  // namespace redis
template <>
const char* ConnectionTraits<REDIS>::GetBasedOn() {
  return "hiredis";
}

template <>
const char* ConnectionTraits<REDIS>::GetVersionApi() {
  return HIREDIS_VERSION;
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::Connect(
    const redis::RConfig& config,
    redis::NativeConnection** hout) {
  redis::NativeConnection* context = nullptr;
  common::Error err = redis::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  // redisEnableKeepAlive(context);
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::Disconnect(
    redis::NativeConnection** handle) {
  redis::NativeConnection* lhandle = *handle;
  if (lhandle) {
    redisFree(lhandle);
  }
  lhandle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::IsConnected(redis::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const ConstantCommandsArray& CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::GetCommands() {
  return redis::g_commands;
}

}  // namespace internal

namespace redis {
namespace {

common::Error ValueFromReplay(redisReply* r, common::Value** out) {
  if (!out || !r) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      *out = common::Value::CreateNullValue();
      break;
    }
    case REDIS_REPLY_ERROR: {
      if (common::strcasestr(r->str, "NOAUTH")) {  //"NOAUTH Authentication
                                                   // required."
      }
      std::string str(r->str, r->len);
      return common::make_error(str);
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      std::string str(r->str, r->len);
      *out = common::Value::CreateStringValue(str);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      *out = common::Value::CreateLongLongIntegerValue(r->integer);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::CreateArrayValue();
      for (size_t i = 0; i < r->elements; ++i) {
        common::Value* val = NULL;
        common::Error err = ValueFromReplay(r->element[i], &val);
        if (err) {
          delete arv;
          return err;
        }
        arv->Append(val);
      }
      *out = arv;
      break;
    }
    default: { return common::make_error(common::MemSPrintf("Unknown reply type: %d", r->type)); }
  }

  return common::Error();
}

common::Error PrintRedisContextError(redisContext* context) {
  if (!context) {
    DNOTREACHED();
    return common::make_error("Not connected");
  }

  return common::make_error(common::MemSPrintf("Error: %s", context->errstr));
}

common::Error ExecRedisCommand(redisContext* c,
                               int argc,
                               const char** argv,
                               const size_t* argvlen,
                               redisReply** out_reply) {
  if (!c) {
    DNOTREACHED();
    return common::make_error("Not connected");
  }

  if (argc <= 0 || !argv || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  int res = redisAppendCommandArgv(c, argc, argv, argvlen);
  if (res == REDIS_ERR) {
    DNOTREACHED();
    return PrintRedisContextError(c);
  }

  void* reply = NULL;
  res = redisGetReply(c, &reply);
  if (res == REDIS_ERR) {
    /* Filter cases where we should reconnect */
    if (c->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error("Needed reconnect.");
    }
    if (c->err == REDIS_ERR_EOF) {
      return common::make_error("Needed reconnect.");
    }

    return PrintRedisContextError(c);
  }

  redisReply* rreply = static_cast<redisReply*>(reply);
  if (rreply->type == REDIS_REPLY_ERROR) {
    std::string str(rreply->str, rreply->len);
    freeReplyObject(rreply);
    return common::make_error(str);
  }

  *out_reply = rreply;
  return common::Error();
}

common::Error ExecRedisCommand(redisContext* c, command_buffer_t command, redisReply** out_reply) {
  if (command.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  int argc = 0;
  sds* argv = sdssplitargslong(command.data(), &argc);
  if (!argv) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  void* raw_argvlen_ptr = malloc(static_cast<size_t>(argc) * sizeof(size_t));
  size_t* argvlen = reinterpret_cast<size_t*>(raw_argvlen_ptr);
  for (int i = 0; i < argc; ++i) {
    argvlen[i] = sdslen(argv[i]);
  }

  common::Error err = ExecRedisCommand(c, argc, const_cast<const char**>(argv), argvlen, out_reply);
  sdsfreesplitres(argv, argc);
  free(raw_argvlen_ptr);
  return err;
}

common::Error ExecRedisCommand(redisContext* c, const commands_args_t& argv, redisReply** out_reply) {
  if (argv.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  const char** argvc = static_cast<const char**>(calloc(sizeof(const char*), argv.size()));
  int argcc = 0;
  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argv.size() * sizeof(size_t)));
  for (size_t i = 0; i < argv.size(); ++i) {
    argvc[i] = argv[i].data();
    argvlen[i] = argv[i].size();
    argcc++;
  }

  common::Error err = ExecRedisCommand(c, argcc, argvc, argvlen, out_reply);
  free(argvlen);
  free(argvc);
  return err;
}

common::Error AuthContext(redisContext* context, const std::string& auth_str) {
  if (auth_str.empty()) {
    return common::Error();
  }

  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(context, {"AUTH", auth_str}, &reply);
  if (err) {
    return err;
  }
  freeReplyObject(reply);
  return common::Error();
}

}  // namespace

RConfig::RConfig(const Config& config, const SSHInfo& sinfo) : Config(config), ssh_info(sinfo) {}

RConfig::RConfig() : Config(), ssh_info() {}

common::Error CreateConnection(const RConfig& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  redisContext* lcontext = NULL;
  bool is_local = !config.hostsocket.empty();

  if (is_local) {
    const char* hostsocket = config.hostsocket.empty() ? NULL : config.hostsocket.c_str();
    lcontext = redisConnectUnix(hostsocket);
  } else {
    SSHInfo sinfo = config.ssh_info;
    std::string host_str = config.host.GetHost();
    const char* host = host_str.empty() ? NULL : host_str.c_str();
    bool is_ssl = config.is_ssl;
    uint16_t port = config.host.GetPort();
    const char* username = sinfo.user_name.empty() ? NULL : sinfo.user_name.c_str();
    const char* password = sinfo.password.empty() ? NULL : sinfo.password.c_str();
    common::net::HostAndPort ssh_host = sinfo.host;
    std::string ssh_host_str = ssh_host.GetHost();
    const char* ssh_address = ssh_host_str.empty() ? NULL : ssh_host_str.c_str();
    int ssh_port = ssh_host.GetPort();
    int curM = sinfo.current_method;
    const char* public_key = sinfo.public_key.empty() ? NULL : sinfo.public_key.c_str();
    const char* private_key = sinfo.private_key.empty() ? NULL : sinfo.private_key.c_str();
    const char* passphrase = sinfo.passphrase.empty() ? NULL : sinfo.passphrase.c_str();
    lcontext = redisConnect(host, port, ssh_address, ssh_port, username, password, public_key, private_key, passphrase,
                            is_ssl, curM);
  }

  if (!lcontext) {
    if (is_local) {
      return common::make_error(common::MemSPrintf("Could not connect to Redis at %s : no context", config.hostsocket));
    }
    std::string host_str = common::ConvertToString(config.host);
    return common::make_error(common::MemSPrintf("Could not connect to Redis at %s : no context", host_str));
  }

  if (lcontext->err) {
    std::string buff;
    if (is_local) {
      buff = common::MemSPrintf("Could not connect to Redis at %s : %s", config.hostsocket, lcontext->errstr);
    } else {
      std::string host_str = common::ConvertToString(config.host);
      buff = common::MemSPrintf("Could not connect to Redis at %s : %s", host_str, lcontext->errstr);
    }
    redisFree(lcontext);
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const RConfig& rconfig) {
  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error DiscoveryClusterConnection(const RConfig& rconfig, std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_inval();
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  /* Send the GET CLUSTER command. */
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SERVER_TYPE));
  if (!reply) {
    err = common::make_error("I/O error");
    redisFree(context);
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    err = makeDiscoveryClusterInfo(rconfig.host, std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error(std::string(reply->str, reply->len));
  } else {
    DNOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error DiscoverySentinelConnection(const RConfig& rconfig, std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_inval();
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_MASTERS));
  if (!masters_reply) {
    redisFree(context);
    return common::make_error("I/O error");
  }

  for (size_t i = 0; i < masters_reply->elements; ++i) {
    redisReply* master_info = masters_reply->element[i];
    ServerCommonInfo sinf;
    common::Error lerr = MakeServerCommonInfo(master_info, &sinf);
    if (lerr) {
      continue;
    }

    const char* master_name = sinf.name.c_str();
    ServerDiscoverySentinelInfoSPtr sent(new DiscoverySentinelInfo(sinf));
    infos->push_back(sent);
    /* Send the GET SLAVES command. */
    redisReply* reply =
        reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_SLAVES_PATTERN_1ARGS_S, master_name));
    if (!reply) {
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error("I/O error");
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
      for (size_t j = 0; j < reply->elements; ++j) {
        redisReply* server_info = reply->element[j];
        ServerCommonInfo slsinf;
        lerr = MakeServerCommonInfo(server_info, &slsinf);
        if (lerr) {
          continue;
        }
        ServerDiscoverySentinelInfoSPtr lsent(new DiscoverySentinelInfo(slsinf));
        infos->push_back(lsent);
      }
    } else if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error(std::string(reply->str, reply->len));
    } else {
      DNOTREACHED();
    }
    freeReplyObject(reply);
  }

  freeReplyObject(masters_reply);
  redisFree(context);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())), is_auth_(false), cur_db_(-1) {}

bool DBConnection::IsAuthenticated() const {
  if (!base_class::IsAuthenticated()) {
    return false;
  }

  return is_auth_;
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err) {
    return err;
  }

  /* Do AUTH and select the right DB. */
  err = Auth(config->auth);
  if (err) {
    return err;
  }

  int db_num = config->db_num;
  err = Select(common::ConvertToString(db_num), NULL);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::Disconnect() {
  cur_db_ = -1;
  is_auth_ = false;
  return base_class::Disconnect();
}

std::string DBConnection::GetCurrentDBName() const {
  if (IsAuthenticated()) {
    return common::ConvertToString(cur_db_);
  }

  DNOTREACHED();
  return base_class::GetCurrentDBName();
}

/* Sends SYNC and reads the number of bytes in the payload.
 * Used both by
 * slaveMode() and getRDB(). */
common::Error DBConnection::SendSync(unsigned long long* payload) {
  if (!payload) {
    DNOTREACHED();
    return common::make_error_inval();
  }
  /* To start we need to send the SYNC command and return
   * the payload.
   * The hiredis client lib does not understand this part of
   * the protocol
   * and we don't want to mess with its buffers, so
   * everything is performed
   * using direct low-level I/O. */
  char buf[4096], *p;

  /* Send the SYNC command. */
  ssize_t nwrite = 0;
  if (redisWriteFromBuffer(connection_.handle_, "SYNC\r\n", &nwrite) == REDIS_ERR) {
    return common::make_error("Error writing to master");
  }

  /* Read $<payload>\r\n, making sure to read just up to
   * "\n" */
  p = buf;
  while (1) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, p, 1, &nread);
    if (res == REDIS_ERR) {
      return common::make_error("Error reading bulk length while SYNCing");
    }

    if (!nread) {
      continue;
    }

    if (*p == '\n' && p != buf) {
      break;
    }
    if (*p != '\n') {
      p++;
    }
  }
  *p = '\0';
  if (buf[0] == '-') {
    std::string buf2 = common::MemSPrintf("SYNC with master failed: %s", buf);
    return common::make_error(buf2);
  }

  *payload = strtoull(buf + 1, NULL, 10);
  return common::Error();
}

common::Error DBConnection::SlaveMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  unsigned long long payload = 0;
  err = SendSync(&payload);
  if (err) {
    return err;
  }

  char buf[1024];
  /* Discard the payload. */
  while (payload) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, buf, (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error("Error reading RDB payload while SYNCing");
    }
    payload -= nread;
  }

  /* Now we can use hiredis to read the incoming protocol.
   */
  while (!IsInterrupted()) {
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::JsonSetImpl(const NDbKValue& key, NDbKValue* added_key) {
  command_buffer_t set_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->CreateKeyCommand(key, &set_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, set_cmd, &reply);
  if (err) {
    return err;
  }

  *added_key = key;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::JsonGetImpl(const NKey& key, NDbKValue* loaded_key) {
  command_buffer_t get_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->LoadKeyCommand(key, JsonValue::TYPE_JSON, &get_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, get_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_NIL) {
    // key_t key_str = key.GetKey();
    return GenerateError(REDIS_JSON_MODULE_COMMAND("GET"), "key not found.");
  }

  CHECK(reply->type == REDIS_REPLY_STRING) << "Unexpected replay type: " << reply->type;
  common::Value* val = new JsonValue(reply->str);
  *loaded_key = NDbKValue(key, NValue(val));
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  const command_buffer_t pattern_result = core::internal::GetKeysPattern(cursor_in, pattern, count_keys);
  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, pattern_result, &reply);
  if (err) {
    return err;
  }

  if (reply->type != REDIS_REPLY_ARRAY) {
    freeReplyObject(reply);
    return common::make_error("I/O error");
  }

  common::Value* val = nullptr;
  err = ValueFromReplay(reply, &val);
  if (err) {
    delete val;
    freeReplyObject(reply);
    return err;
  }

  common::ArrayValue* arr = static_cast<common::ArrayValue*>(val);
  CHECK_EQ(2, arr->GetSize());
  std::string cursor_out_str;
  if (!arr->GetString(0, &cursor_out_str)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error("I/O error");
  }

  common::ArrayValue* arr_keys = nullptr;
  if (!arr->GetList(1, &arr_keys)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error("I/O error");
  }

  for (size_t i = 0; i < arr_keys->GetSize(); ++i) {
    std::string key;
    if (arr_keys->GetString(i, &key)) {
      keys_out->push_back(key);
    }
  }

  uint64_t lcursor_out;
  if (!common::ConvertFromString(cursor_out_str, &lcursor_out)) {
    return common::make_error_inval();
  }

  *cursor_out = lcursor_out;
  delete val;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  UNUSED(key_start);
  UNUSED(key_end);
  UNUSED(limit);
  UNUSED(ret);
  return ICommandTranslator::NotSupported(DB_KEYS_COMMAND);
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DBSIZE));

  if (!reply || reply->type != REDIS_REPLY_INTEGER) {
    return common::make_error("Couldn't determine " DB_DBKCOUNT_COMMAND "!");
  }

  /* Grab the number of keys and free our reply */
  *size = static_cast<size_t>(reply->integer);
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DB_FLUSHDB_COMMAND));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  int num;
  if (!common::ConvertFromString(name, &num)) {
    return common::make_error_inval();
  }

  command_buffer_t select_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->SelectDBCommand(common::ConvertToString(num), &select_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, select_cmd, &reply);
  if (err) {
    return err;
  }

  connection_.config_->db_num = num;
  cur_db_ = num;
  size_t sz = 0;
  err = DBkcount(&sz);
  DCHECK(!err);
  DataBaseInfo* linfo = new DataBaseInfo(common::ConvertToString(num), true, sz);
  *info = linfo;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    command_buffer_t del_cmd;
    redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
    common::Error err = tran->DeleteKeyCommand(key, &del_cmd);
    if (err) {
      return err;
    }

    redisReply* reply = NULL;
    err = ExecRedisCommand(connection_.handle_, del_cmd, &reply);
    if (err) {
      return err;
    }

    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
      deleted_keys->push_back(key);
    }
    freeReplyObject(reply);
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  command_buffer_t set_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->CreateKeyCommand(key, &set_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, set_cmd, &reply);
  if (err) {
    return err;
  }

  *added_key = key;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  command_buffer_t get_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->LoadKeyCommand(key, common::Value::TYPE_STRING, &get_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, get_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_NIL) {
    // key_t key_str = key.GetKey();
    return GenerateError(DB_GET_KEY_COMMAND, "key not found.");
  }

  CHECK(reply->type == REDIS_REPLY_STRING) << "Unexpected replay type: " << reply->type;
  common::Value* val = common::Value::CreateStringValue(reply->str);
  *loaded_key = NDbKValue(key, NValue(val));
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t rename_cmd;
  common::Error err = tran->RenameKeyCommand(key, key_t(new_key), &rename_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, rename_cmd, &reply);
  if (err) {
    return err;
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  key_t key_str = key.GetKey();
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t ttl_cmd;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &ttl_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->integer == 0) {
    return GenerateError(DB_SET_TTL_COMMAND, "key does not exist or the timeout could not be set.");
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t ttl_cmd;
  common::Error err = tran->LoadKeyTTLCommand(key, &ttl_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type != REDIS_REPLY_INTEGER) {
    freeReplyObject(reply);
    return common::make_error("TTL command internal error");
  }

  *ttl = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::ModuleLoadImpl(const ModuleInfo& module) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t module_load_cmd;
  common::Error err = tran->ModuleLoadCommand(module, &module_load_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, module_load_cmd, &reply);
  if (err) {
    return err;
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::ModuleUnLoadImpl(const ModuleInfo& module) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t module_unload_cmd;
  common::Error err = tran->ModuleUnloadCommand(module, &module_unload_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, module_unload_cmd, &reply);
  if (err) {
    return err;
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DB_QUIT_COMMAND));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  common::Error err = Disconnect();
  UNUSED(err);
  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObject* out, redisReply* r) {
  if (!out || !r) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Value* out_val = nullptr;
  common::Error err = ValueFromReplay(r, &out_val);
  if (err) {
    if (err->GetDescription() == "NOAUTH") {  //"NOAUTH Authentication
                                              // required."
      is_auth_ = false;
    }
    return err;
  }

  FastoObject* obj = new FastoObject(out, out_val, GetDelimiter());
  out->AddChildren(obj);
  return common::Error();
}

common::Error DBConnection::CliReadReply(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsConnected();
  if (err) {
    return err;
  }

  void* _reply = NULL;
  if (redisGetReply(connection_.handle_, &_reply) != REDIS_OK) {
    /* Filter cases where we should reconnect */
    if (connection_.handle_->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error("Needed reconnect.");
    }
    if (connection_.handle_->err == REDIS_ERR_EOF) {
      return common::make_error("Needed reconnect.");
    }

    return PrintRedisContextError(connection_.handle_); /* avoid compiler warning */
  }

  redisReply* reply = static_cast<redisReply*>(_reply);
  common::Error er = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return er;
}

common::Error DBConnection::ExecuteAsPipeline(const std::vector<FastoObjectCommandIPtr>& cmds,
                                              void (*log_command_cb)(FastoObjectCommandIPtr command)) {
  if (cmds.empty()) {
    return common::make_error("Invalid input command");
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  // start piplene mode
  std::vector<FastoObjectCommandIPtr> valid_cmds;
  for (size_t i = 0; i < cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    command_buffer_t command = cmd->GetInputCommand();
    if (command.empty()) {
      continue;
    }

    if (log_command_cb) {
      log_command_cb(cmd);
    }

    int argc = 0;
    sds* argv = sdssplitargslong(command.data(), &argc);
    if (argv) {
      if (isPipeLineCommand(argv[0])) {
        valid_cmds.push_back(cmd);
        void* raw_argvlen_ptr = malloc(static_cast<size_t>(argc) * sizeof(size_t));
        size_t* argvlen = reinterpret_cast<size_t*>(raw_argvlen_ptr);
        for (int i = 0; i < argc; ++i) {
          argvlen[i] = sdslen(argv[i]);
        }
        redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
        free(raw_argvlen_ptr);
      }
      sdsfreesplitres(argv, argc);
    }
  }

  for (size_t i = 0; i < valid_cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::Error err = CliReadReply(cmd.get());
    if (err) {
      return err;
    }
  }
  // end piplene

  return common::Error();
}

common::Error DBConnection::CommonExec(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return err;
}

common::Error DBConnection::Auth(const std::string& password) {
  common::Error err = TestIsConnected();
  if (err) {
    return err;
  }

  err = AuthContext(connection_.handle_, password);
  if (err) {
    is_auth_ = false;
    return err;
  }

  is_auth_ = true;
  return common::Error();
}

common::Error DBConnection::Monitor(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::Subscribe(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::SetEx(const NDbKValue& key, ttl_t ttl) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setex_cmd;
  err = tran->SetEx(key, ttl, &setex_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setex_cmd, &reply);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(key);
  }
  if (client_) {
    client_->OnChangedKeyTTL(key.GetKey(), ttl);
  }
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetNX(const NDbKValue& key, long long* result) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setnx_cmd;
  err = tran->SetNX(key, &setnx_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setnx_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_ && reply->integer) {
      client_->OnAddedKey(key);
    }

    *result = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lpush(const NKey& key, NValue arr, long long* list_len) {
  if (!arr || arr->GetType() != common::Value::TYPE_ARRAY || !list_len) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rarr(key, arr);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lpush_cmd;
  err = tran->CreateKeyCommand(rarr, &lpush_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lpush_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rarr);
    }
    *list_len = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lrange(const NKey& key, int start, int stop, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lrange_cmd;
  err = tran->Lrange(key, start, stop, &lrange_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lrange_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = ValueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    *loaded_key = NDbKValue(key, NValue(val));
    if (client_) {
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Sadd(const NKey& key, NValue set, long long* added) {
  if (!set || set->GetType() != common::Value::TYPE_SET || !added) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rset(key, set);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t sadd_cmd;
  err = tran->CreateKeyCommand(rset, &sadd_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, sadd_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Smembers(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t smembers_cmd;
  err = tran->Smembers(key, &smembers_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, smembers_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = ValueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to set");
    }

    common::SetValue* set = common::Value::CreateSetValue();
    for (size_t i = 0; i < arr->GetSize(); ++i) {
      common::Value* lval = nullptr;
      if (arr->Get(i, &lval)) {
        set->Insert(lval->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(set));
    if (client_) {
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zadd(const NKey& key, NValue scores, long long* added) {
  if (!scores || scores->GetType() != common::Value::TYPE_ZSET || !added) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rzset(key, scores);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zadd_cmd;
  err = tran->CreateKeyCommand(rzset, &zadd_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zadd_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rzset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zrange(const NKey& key, int start, int stop, bool withscores, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zrange;
  err = tran->Zrange(key, start, stop, withscores, &zrange);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zrange, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = ValueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    if (!withscores) {
      *loaded_key = NDbKValue(key, NValue(val));
      if (client_) {
        client_->OnLoadedKey(*loaded_key);
      }
      freeReplyObject(reply);
      return common::Error();
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to zset");
    }

    common::ZSetValue* zset = common::Value::CreateZSetValue();
    for (size_t i = 0; i < arr->GetSize(); i += 2) {
      common::Value* lmember = nullptr;
      common::Value* lscore = nullptr;
      if (arr->Get(i, &lmember) && arr->Get(i + 1, &lscore)) {
        zset->Insert(lscore->DeepCopy(), lmember->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(zset));
    if (client_) {
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hmset(const NKey& key, NValue hash) {
  if (!hash || hash->GetType() != common::Value::TYPE_HASH) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rhash(key, hash);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hmset_cmd;
  err = tran->CreateKeyCommand(rhash, &hmset_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hmset_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_STATUS) {
    if (client_) {
      client_->OnAddedKey(rhash);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hgetall(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hgetall_cmd;
  err = tran->Hgetall(key, &hgetall_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hgetall_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = ValueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to hash");
    }

    common::HashValue* hash = common::Value::CreateHashValue();
    for (size_t i = 0; i < arr->GetSize(); i += 2) {
      common::Value* lkey = nullptr;
      common::Value* lvalue = nullptr;
      if (arr->Get(i, &lkey) && arr->Get(i + 1, &lvalue)) {
        hash->Insert(lkey->DeepCopy(), lvalue->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(hash));
    if (client_) {
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Decr(const NKey& key, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string decr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->Decr(key, &decr_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decr_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::DecrBy(const NKey& key, int dec, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string decrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->DecrBy(key, dec, &decrby_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decrby_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Incr(const NKey& key, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->Incr(key, &incr_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incr_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrBy(const NKey& key, int inc, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->IncrBy(key, inc, &incrby_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrby_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrByFloat(const NKey& key, double inc, std::string* str_incr) {
  if (!str_incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incrfloat_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->IncrByFloat(key, inc, &incrfloat_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrfloat_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    std::string str(reply->str, reply->len);
    if (client_) {
      NValue val(common::Value::CreateStringValue(str));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *str_incr = str;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::GraphQuery(const commands_args_t& argv, FastoObject* out) {
  return CommonExec(argv, out);
}

common::Error DBConnection::GraphExplain(const commands_args_t& argv, FastoObject* out) {
  return CommonExec(argv, out);
}

common::Error DBConnection::GraphDelete(const commands_args_t& argv, FastoObject* out) {
  return CommonExec(argv, out);
}

common::Error DBConnection::JsonSet(const NDbKValue& key, NDbKValue* added_key) {
  if (!added_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = JsonSetImpl(key, added_key);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(*added_key);
  }

  return common::Error();
}

common::Error DBConnection::JsonGet(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = JsonGetImpl(key, loaded_key);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnLoadedKey(*loaded_key);
  }

  return common::Error();
}

bool DBConnection::IsInternalCommand(const std::string& command_name) {
  if (command_name.empty()) {
    return false;
  }

  for (size_t i =0; i < g_internal_commands.size(); ++i) {
    const CommandHolder& cmd = g_internal_commands[i];
    if (cmd.IsEqualFirstName(command_name)){
      return true;
    }
  }

  return false;
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
