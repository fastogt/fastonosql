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

#define REDIS_MODULE_COMMAND_GENERATE(MODULE, COMMAND) MODULE "." COMMAND

#define REDIS_GRAPH_MODULE "GRAPH"
#define REDIS_GRAPH_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_GRAPH_MODULE, COMMAND)

#define REDIS_SEARCH_MODULE "FT"
#define REDIS_SEARCH_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_SEARCH_MODULE, COMMAND)

#define REDIS_JSON_MODULE "JSON"
#define REDIS_JSON_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_JSON_MODULE, COMMAND)

namespace fastonosql {
namespace core {
namespace {
inline commands_args_t ExpandCommand(std::initializer_list<command_buffer_t> list, commands_args_t argv) {
  argv.insert(argv.begin(), list);
  return argv;
}
}  // namespace
namespace redis {

const internal::ConstantCommandsArray g_commands = {
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
    CommandHolder(REDIS_SEARCH_MODULE_COMMAND("SEARCH"),
                  "<index> <query> [NOCONTENT] [LIMIT offset num]",
                  "Search the index with a textual query, returning either documents or just ids.",
                  PROJECT_VERSION_GENERATE(3, 4, 0),
                  REDIS_SEARCH_MODULE_COMMAND("SEARCH") " idx \"hello world\" LIMIT 0 1",
                  5,
                  1,
                  CommandInfo::Extended,
                  &CommandsApi::FtSearch),
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
                  &CommandsApi::JsonResp)};

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

common::Error CommandsApi::ModuleLoad(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MODULE", "LOAD"}, argv), out);
}

common::Error CommandsApi::ModuleUnLoad(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(ExpandCommand({"MODULE", "UNLOAD"}, argv), out);
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
  return red->GraphDelete(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("ADD")}, argv), out);
}

common::Error CommandsApi::FtCreate(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("CREATE")}, argv), out);
}

common::Error CommandsApi::FtSearch(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_SEARCH_MODULE_COMMAND("SEARCH")}, argv), out);
}

common::Error CommandsApi::JsonDel(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("DEL")}, argv), out);
}

common::Error CommandsApi::JsonGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("GET")}, argv), out);
}

common::Error CommandsApi::JsonMget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("MGET")}, argv), out);
}

common::Error CommandsApi::JsonSet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("SET")}, argv), out);
}

common::Error CommandsApi::JsonType(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("TYPE")}, argv), out);
}

common::Error CommandsApi::JsonNumIncrBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("NUMINCRBY")}, argv), out);
}

common::Error CommandsApi::JsonNumMultBy(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("NUMMULTBY")}, argv), out);
}

common::Error CommandsApi::JsonStrAppend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("STRAPPEND")}, argv), out);
}

common::Error CommandsApi::JsonStrlen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("STRLEN")}, argv), out);
}

common::Error CommandsApi::JsonArrAppend(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRAPPEND")}, argv), out);
}

common::Error CommandsApi::JsonArrIndex(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRINDEX")}, argv), out);
}

common::Error CommandsApi::JsonArrInsert(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRINSERT")}, argv), out);
}

common::Error CommandsApi::JsonArrLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRLEN")}, argv), out);
}

common::Error CommandsApi::JsonArrPop(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRPOP")}, argv), out);
}

common::Error CommandsApi::JsonArrTrim(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("ARRTRIM")}, argv), out);
}

common::Error CommandsApi::JsonObjKeys(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("OBJKEYS")}, argv), out);
}

common::Error CommandsApi::JsonObjLen(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("OBJLEN")}, argv), out);
}

common::Error CommandsApi::JsonDebug(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("DEBUG")}, argv), out);
}

common::Error CommandsApi::JsonForget(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("FORGET")}, argv), out);
}

common::Error CommandsApi::JsonResp(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->GraphDelete(ExpandCommand({REDIS_JSON_MODULE_COMMAND("RESP")}, argv), out);
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
