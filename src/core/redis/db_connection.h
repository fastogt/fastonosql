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

#include <stddef.h>                     // for size_t

#include <vector>                       // for vector

#include "common/error.h"               // for Error
#include "common/macros.h"              // for PROJECT_VERSION_GENERATE, etc

#include "core/command_handler.h"       // for CommandHandler
#include "core/command_holder.h"        // for CommandHolder, etc
#include "core/ssh_info.h"              // for SSHInfo
#include "core/types.h"                 // for IDataBaseInfo (ptr only), etc
#include "core/db_connection.h"

#include "core/redis/connection_settings.h"
#include "core/redis/config.h"

#include "global/global.h"              // for FastoObject (ptr only), etc

#define INFO_REQUEST "INFO"
#define GET_SERVER_TYPE "CLUSTER NODES"
#define GET_SENTINEL_MASTERS "SENTINEL MASTERS"
#define GET_SENTINEL_SLAVES_PATTERN_1ARGS_S "SENTINEL SLAVES %s"

#define LATENCY_REQUEST "LATENCY"
#define RDM_REQUEST "RDM"
#define SYNC_REQUEST "SYNC"
#define FIND_BIG_KEYS_REQUEST "FIND_BIG_KEYS"
#define STAT_MODE_REQUEST "STAT"
#define SCAN_MODE_REQUEST "SCAN"

struct redisContext;
struct redisReply;

namespace fastonosql {
namespace core {
namespace redis {

common::Error common_exec(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error common_exec_off2(CommandHandler* handler, int argc, char** argv, FastoObject* out);

common::Error auth(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error select(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error connect(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error monitor(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error subscribe(CommandHandler* handler, int argc, char** argv, FastoObject* out);
common::Error sync(CommandHandler* handler, int argc, char** argv, FastoObject* out);

static const std::vector<CommandHolder> redisCommands = {
  CommandHolder("APPEND", "<key> <value>",
              "Append a value to a key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("AUTH", "<password>",
              "Authenticate to the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &auth),
  CommandHolder("BGREWRITEAOF", "-",
              "Asynchronously rewrite the append-only file", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("BGSAVE", "-",
              "Asynchronously save the dataset to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("BITCOUNT","<key> [start] [end]",
              "Count set bits in a string", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 2, &common_exec),
  CommandHolder("BITOP", "<operation> <destkey> <key> [key ...]",
              "Perform bitwise operations between strings", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 2, &common_exec),
  CommandHolder("BITPOS", "<key> <bit> [start] [end]",
              "Find first bit set or clear in a string", PROJECT_VERSION_GENERATE(2,8,7), UNDEFINED_EXAMPLE_STR, 2, 2, &common_exec),
  CommandHolder("BLPOP", "<key> [key ...] timeout",
              "Remove and get the first element in a list, or block until one is available", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("BRPOP", "<key> [key ...] timeout",
              "Remove and get the last element in a list, or block until one is available", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("BRPOPLPUSH", "source destination timeout",
              "Pop a value from a list, push it to another list and return it; or block until one is available", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),

  CommandHolder("CLIENT GETNAME", "-",
              "Get the current connection name", PROJECT_VERSION_GENERATE(2,6,9), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CLIENT KILL", "[ip:port] [ID client-id] [TYPE normal|master|slave|pubsub] [ADDR ip:port] [SKIPME yes/no]",
              "Kill the connection of a client", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLIENT LIST", "-",
              "Get the list of client connections", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CLIENT PAUSE", "<timeout>",
              "Stop processing commands from clients for some time", PROJECT_VERSION_GENERATE(2,9,50), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLIENT REPLY", "ON|OFF|SKIP",
              "Instruct the server whether to reply to commands", PROJECT_VERSION_GENERATE(3,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLIENT SETNAME", "<connection-name>",
              "Set the current connection name", PROJECT_VERSION_GENERATE(2,6,9), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),

  CommandHolder("CLUSTER ADDSLOTS", "<slot> [slot ...]",
              "Assign new hash slots to receiving node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, INFINITE_COMMAND_ARGS, &common_exec_off2),
  CommandHolder("CLUSTER COUNT-FAILURE-REPORTS", "<node-id>",
              "Return the number of failure reports active for a given node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER COUNTKEYSINSLOT", "<slot>",
              "Return the number of local keys in the specified hash slot", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER DELSLOTS", "<slot> [slot ...]",
              "Set hash slots as unbound in receiving node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, INFINITE_COMMAND_ARGS, &common_exec_off2),
  CommandHolder("CLUSTER FAILOVER", "[FORCE|TAKEOVER]",
              "Forces a slave to perform a manual failover osyncf its master.", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec_off2),
  CommandHolder("CLUSTER FORGET", "<node-id>",
              "Remove a node from the nodes table", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER GETKEYSINSLOT", "<slot> <count>",
              "Return local key names in the specified hash slot", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec_off2),
  CommandHolder("CLUSTER INFO", "-",
              "Provides info about Redis Cluster node state", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CLUSTER KEYSLOT", "<key>",
              "Returns the hash slot of the specified key", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER MEET", "<ip> <port>",
              "Force a node cluster to handshake with another node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec_off2),
  CommandHolder("CLUSTER NODES", "-",
              "Get Cluster config for the node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CLUSTER REPLICATE", "<node-id>",
              "Reconfigure a node as a slave of the specified master node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER RESET", "[HARD|SOFT]",
              "Reset a Redis Cluster node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec_off2),
  CommandHolder("CLUSTER SAVECONFIG", "-",
              "Forces the node to save cluster state on disk", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CLUSTER SET-CONFIG-EPOCH", "<config-epoch>",
              "Set the configuration epoch in a new node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER SETSLOT", "<slot> IMPORTING|MIGRATING|STABLE|NODE [node-id]",
              "Bind a hash slot to a specific node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 2, 1, &common_exec_off2),
  CommandHolder("CLUSTER SLAVES", "<node-id>",
              "Licommon_execst slave nodes of the specified master node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CLUSTER SLOTS", "-",
              "Get array of Cluster slot to node mappings", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),

  CommandHolder("COMMAND", "-",
              "Get array of Redis command details", PROJECT_VERSION_GENERATE(2,8,13), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("COMMAND COUNT", "-",
              "Get total number of Redis commands", PROJECT_VERSION_GENERATE(2,8,13), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("COMMAND GETKEYS", "-",
              "Extract keys given a full Redis command", PROJECT_VERSION_GENERATE(2,8,13), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("COMMAND INFO", "command-name [command-name ...]",
              "Get array of specific Redis command details", PROJECT_VERSION_GENERATE(2,8,13), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),

  CommandHolder("CONFIG GET", "<parameter>",
              "Get the value of a configuration parameter", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("CONFIG RESETSTAT", "-",
              "Reset the stats returned by INFO", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CONFIG REWRITE", "-",
              "Rewrite the configuration file with the in memory configuration", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("CONFIG SET", "<parameter> <value>",
              "Set a configuration parameter to the given value", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec_off2),

  CommandHolder("CONNECT", "<ip> <port>",
                "Open connection", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &connect),

  CommandHolder("DBSIZE", "-",
              "Return the number of keys in the selected database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),

  CommandHolder("DEBUG OBJECT", "<key>",
              "Get debugging information about a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("DEBUG SEGFAULT", "-",
              "Make the server crash", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),

  CommandHolder("DECR", "<key>",
              "Decrement the integer value of a key by one", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("DECRBY", "<key> <decrement>",
              "Decrement the integer value of a key by the given number", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("DEL", "<key> [key ...]",
              "Delete a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("DISCARD", "-",
              "Discard all commands issued after MULTI", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("DUMP", "<key>",
              "Return a serialized version of the value stored at the specified key.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("ECHO", "<message>",
              "Echo the given string", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("EVAL", "script numkeys <key> [key ...] <arg> [arg ...]",
              "Execute a Lua script server side", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("EVALSHA", "sha1 numkeys <key> [key ...] <arg> [arg ...]",
              "Execute a Lua script server side", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("EXEC", "-",
              "Execute all commands issued after MULTI", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("EXISTS",
              "key [key ...]", "Determine if a key exists", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("EXPIRE", "<key> <seconds>",
              "Set a key's time to live in seconds", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("EXPIREAT", "<key> <timestamp>",
              "Set the expiration for a key as a UNIX timestamp", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("FLUSHALL", "-",
              "Remove all keys from all databases", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("FLUSHDB", "-",
              "Remove all keys from the current database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("GEOADD", "key longitude latitude member [longitude latitude member ...",
              "Add one or more geospatial items in the geospatial index represented using a sorted set", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &common_exec),
  CommandHolder("GEODIST", "key member1 member2 [unit]",
              "Returns the distance between two members of a geospatial index", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("GEOHASH", "key member [member ...]",
              "Returns members of a geospatial index as standard geohash strings", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("GEOPOS", "key member [member ...]",
              "Returns longitude and latitude of members of a geospatial index", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),

  CommandHolder("GEORADIUS", "key longitude latitude radius m|km|ft|mi [WITHCOORD] [WITHDIST] [WITHHASH] [COUNT count] [ASC|DESC]",
              "Query a sorted set representing a geospatial index to fetch members matching a given maximum distance from a point", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 5, 0, &common_exec),
  CommandHolder("GEORADIUSBYMEMBER", "key member radius m|km|ft|mi [WITHCOORD] [WITHDIST] [WITHHASH] [COUNT count] [ASC|DESC]",
              "Query a sorted set representing a geospatial index to fetch members matching a given maximum distance from a member", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &common_exec),
  CommandHolder("GET", "<key>",
              "Gecommon_exect the value of a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("GETBIT", "<key> <offset>",
              "Returns the bit value at offset in the string value stored at key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("GETRANGE", "<key> <start> <end>",
              "Get a substring of the string stored at a key", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("GETSET", "<key> <value>",
              "Set the string value of a key and return its old value", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HDEL", "<key> <field> [field ...]",
              "Delete one or more hash fields", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HEXISTS", "<key> <field>",
              "Determine if a hash field exists", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HGET", "<key> <field>",
              "Get the value of a hash field", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HGETALL", "<key>",
              "Get all the fields and values in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("HINCRBY", "<key> <field> <increment>",
              "Increment the integer value of a hash field by the given number", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("HINCRBYFLOAT", "<key> <field> <increment>",
              "Increment the float value of a hash field by the given amount", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("HKEYS", "<key>",
              "Get all the fields in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("HLEN", "<key>",
              "Get the number of fields in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("HMGET", "<key> <field> [field ...]",
              "Get the values of all the given hash fields", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HMSET", "<key> <field> <value> [field value ...]",
              "Set multiple hash fields to multiple values", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("HSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate hash fields and associated values", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 4, &common_exec),
  CommandHolder("HSET", "<key> <field> <value>",
              "Set the string value of a hash field", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("HSETNX", "<key> <field> <value>",
              "Set the value of a hash field, only if the field does not exist", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("HSTRLEN", "<key> <field>",
              "Get the length of the value of a hash field", PROJECT_VERSION_GENERATE(3,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("HVALS", "<key>",
              "Get all the values in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("INCR", "<key>",
              "Increment the integer value of a key by one", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("INCRBY", "<key> <increment>",
              "Increment the integer value of a key by the given amount", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("INCRBYFLOAT", "<key> <increment>",
              "Increment the float value of a key by the given amount", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("INFO", "[section]",
              "Get information and statistics about the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec),
  CommandHolder("KEYS", "<pattern>",
              "Find all keys matching the given pattern", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("LASTSAVE", "-",
              "Get the UNIX time stamp of the last successful save to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("LINDEX", "<key> <index>",
              "Get an element from a list by its index", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("LINSERT", "<key> <BEFORE|AFTER> <pivot value>",
              "Insert an element before or after another element in a list", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("LLEN", "<key>",
              "Get the length of a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("LPOP", "<key>",
              "Remove and get the first element in a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("LPUSH", "<key> <value> [value ...]",
              "Prepend one or multiple values to a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("LPUSHX", "<key> <value>",
              "Prepend a value to a list, only if the list exists", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("LRANGE", "<key> <start> <stop>",
              "Get a range of elements from a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("LREM", "<key> <count> <value>",
              "Remove elements from a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("LSET", "<key> <index> <value>",
              "Set the value of an element in a list by its index", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("LTRIM", "<key> <start> <stop>",
              "Trim a list to the specified range", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("MGET", "<key> [key ...]",
              "Get the values of all the given keys", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("MIGRATE", "<host> <port> <|""> <destination-db> <timeout> [COPY] [REPLACE]",
              "Atomically transfer a key from a Redis instance to another one.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 5, 2, &common_exec),
  CommandHolder("MONITOR", "-",
              "Listen for all requests received by the server in real time", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &monitor),
  CommandHolder("MOVE", "<key> <db>",
              "Move a key to another database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("MSET", "<key> <value> [key value ...]",
              "Set multiple keys to multiple values", PROJECT_VERSION_GENERATE(1,0,1), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("MSETNX", "<key> <value> [key value ...]",
              "Set multiple keys to multiple values, only if none of the keys exist", PROJECT_VERSION_GENERATE(1,0,1), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("MULTI", "-",
              "Mark the start of a transaction block", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("OBJECT", "<subcommand> [arguments [arguments ...]]",
              "Inspect the internals of Redis objects", PROJECT_VERSION_GENERATE(2,2,3), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("PERSIST", "<key>",
              "Remove the expiration from a key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("PEXPIRE", "<key> <milliseconds>",
              "Set a key's time to live in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("PEXPIREAT", "<key> <milliseconds-timestamp>",
              "Set the expiration for a key as a UNIX timestamp specified in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("PFADD", "<key> <element> [element ...]",
              "Adds the specified elements to the specified HyperLogLog.", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("PFCOUNT", "<key> [key ...]",
              "Return the approximated cardinality of the set(s) observed by the HyperLogLog at key(s).", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("PFMERGE", "<destkey> <sourcekey> [sourcekey ...]",
              "Merge N different HyperLogLogs into a single one.", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("PING", "[message]",
              "Ping the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec),
  CommandHolder("PSETEX", "<key> <milliseconds> <value>",
              "Set the value and expiration in milliseconds of a key", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("PSUBSCRIBE", "<pattern> [pattern ...]",
              "Listen for messages published to channels matching the given patterns", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &subscribe),
  CommandHolder("PTTL", "<key>",
              "Get the time to live for a key in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("PUBLISH", "<channel> <message>",
              "Post a message to a channel", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("PUBSUB", "<subcommand> [argument [argument ...]]",
              "Inspect the state of the Pub/Sub subsystem", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("PUNSUBSCRIBE", "[pattern [pattern ...]]",
              "Stop listening for messages posted to channels matching the given patterns", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec),
  CommandHolder("QUIT", "-",
              "Close the connection", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("RANDOMKEY", "-",
              "Return a random key from the keyspace", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("READONLY", "-",
              "Enables read queries for a connection to a cluster slave node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("READWRITE", "-",
              "Disables read queries for a connection to a cluster slave node", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("RENAME", "<key> <newkey>",
              "Rename a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("RENAMENX", "<key> <newkey>",
              "Rename a key, only if the new key does not exist", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("RESTORE", "<key> <ttl> <serialized-value> [REPLACE]",
              "Create a key using the provided serialized value, previously obtained using DUMP.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ROLE", "-",
              "Return the role of the instance in the context of replication", PROJECT_VERSION_GENERATE(2,8,12), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("RPOP", "<key>",
              "Remove and get the last element in a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("RPOPLPUSH", "<source> <destination>",
              "Remove the last element in a list, prepend it to another list and return it", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("RPUSH", "<key> <value> [value ...]",
              "Append one or multiple values to a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("RPUSHX", "<key> <value>",
              "Append a value to a list, only if the list exists", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SADD", "<key> <member> [member ...]",
              "Add one or more members to a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SAVE", "-",
              "Synchronously save the dataset to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("SCAN", "<cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate the keys space", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 1, 4, &common_exec),
  CommandHolder("SCARD", "<key>",
              "Get the number of members in a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),

  CommandHolder("SCRIPT DEBUG", "<YES|SYNC|NO>",
              "Set the debug mode for executed scripts.", PROJECT_VERSION_GENERATE(3,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SCRIPT EXISTS", "script [script ...]",
              "Check existence of scripts in the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SCRIPT FLUSH", "-",
              "Remove all the scripts from the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("SCRIPT KILL", "-",
              "Kill the script currently in execution.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("SCRIPT LOAD", "<script>",
              "Load the specified Lua script into the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),

  CommandHolder("SDIFF", "<key> [key ...]",
              "Subtract multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SDIFFSTORE", "<destination> <key> [key ...]",
              "Subtract multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SELECT", "<index>",
              "Change the selected database for the current connection", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &select),
  CommandHolder("SET", "<key> <value> [EX seconds] [PX milliseconds] [NX|XX]",
              "Set the string value of a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 3, &common_exec),
  CommandHolder("SETBIT", "<key> <offset> <value>",
              "Sets or clears the bit at offset in the string value stored at key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("SETEX", "<key> <seconds> <value>",
              "Set the value and expiration of a key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("SETNX", "<key> <value>",
              "Set the value of a key, only if the key does not exist", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SETRANGE", "<key> <offset> <value>",
              "Overwrite part of a string at key starting at the specified offset", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("SHUTDOWN", "[NOSAVE|SAVE]",
              "Synchronously save the dataset to disk and then shut down the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 1, &common_exec),
  CommandHolder("SINTER", "<key> [key ...]",
              "Intersect multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SINTERSTORE", "<destination> <key> [key ...]",
              "Intersect multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SISMEMBER", "<key> <member>",
              "Determine if a given value is a member of a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SLAVEOF", "<host> <port>",
              "Make the server a slave of another instance, or promote it as master", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SLOWLOG", "<subcommand> [argument]",
              "Manages the Redis slow queries log", PROJECT_VERSION_GENERATE(2,2,12), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SMEMBERS", "<key>",
              "Get all the members in a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SMOVE", "<source> <destination> <member>",
              "Move a member from one set to another", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("SORT", "<key> [BY pattern] [LIMIT offset count] [GET pattern [GET pattern ...]] [ASC|DESC] [ALPHA] [STORE destination]",
              "Sort the elements in a list, set or sorted set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 6, &common_exec),
  CommandHolder("SPOP", "<key> [count]",
              "Remove and return one or multiple random members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 1, &common_exec),
  CommandHolder("SRANDMEMBER",
              "<key> [count]", "Get one or multiple random members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 1, &common_exec),
  CommandHolder("SREM", "<key> <member> [member ...]",
              "Remove one or more members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 1, &common_exec),
  CommandHolder("SSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate Set elements", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 4, &common_exec),
  CommandHolder("STRLEN", "<key>",
              "Get the length of the value stored in a key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SUBSCRIBE", "<channel> [channel ...]",
              "Listen for messages published to the given channels", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &subscribe),
  CommandHolder("SUNION", "<key> [key ...]",
              "Add multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("SUNIONSTORE", "<destination> <key> [key ...]",
              "Add multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("SYNC", "-",
              "Internal command used for replication", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &sync),
  CommandHolder("PSYNC", "-",
              "Internal command used for replication", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &sync),
  CommandHolder("TIME", "-",
              "Return the current server time", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("TTL", "<key>",
              "Get the time to live for a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("TYPE", "<key>",
              "Determine the type stored at key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("UNSUBSCRIBE", "[channel [channel ...]]",
              "Stop listening for messages posted to the given channels", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("UNWATCH", "-",
              "Forget about all watched keys", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec),
  CommandHolder("WAIT", "numslaves timeout",
              "Wait for the synchronous replication of all the write commands sent in the context of the current connection", PROJECT_VERSION_GENERATE(3,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("WATCH", "key [key ...]",
              "Watch the given keys to determine execution of the MULTI/EXEC block", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("ZADD", "<key> [NX|XX] [CH] [INCR] <score> <member> [score member ...]",
              "Add one or more members to a sorted set, or update its score if it already exists", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZCARD", "<key>",
              "Get the number of members in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec),
  CommandHolder("ZCOUNT", "<key> <min> <max>",
              "Count the members in a sorted set with scores within the given values", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZINCRBY", "<key> <increment> <member>",
              "Increment the score of a member in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZINTERSTORE", "<destination> <numkeys> <key> [key ...] [WEIGHTS weight] [AGGREGATE SUM|MIN|MAX]",
              "Intersect multiple sorted sets and store the resulting sorted set in a new key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 2, &common_exec),
  CommandHolder("ZLEXCOUNT", "<key> <min> <max>",
              "Count the number of members in a sorted set between a given lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZRANGE", "<key> <start> <stop> [WITHSCORES]",
              "Return a range of members in a sorted set, by index", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 1, &common_exec),
  CommandHolder("ZRANGEBYLEX", "<key> <min> <max> [LIMIT offset count]",
              "Return a range of members in a sorted set, by lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 3, &common_exec),
  CommandHolder("ZRANGEBYSCORE", "<key> <min> <max> [WITHSCORES] [LIMIT offset count]",
              "Return a range of members in a sorted set, by score", PROJECT_VERSION_GENERATE(1,0,5), UNDEFINED_EXAMPLE_STR, 3, 3, &common_exec),
  CommandHolder("ZRANK", "<key> <member>",
              "Determine the index of a member in a sorted set", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("ZREM", "<key> <member> [member ...]",
              "Remove one or more members from a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("ZREMRANGEBYLEX", "<key> <min> <max>",
              "Remove all members in a sorted set between the given lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZREMRANGEBYRANK", "<key> <start> <stop>",
              "Remove all members in a sorted set within the given indexes", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZREMRANGEBYSCORE", "<key> <min> <max>",
              "Remove all members in a sorted set within the given scores", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec),
  CommandHolder("ZREVRANGE", "<key> <start> <stop> [WITHSCORES]",
              "Return a range of members in a sorted set, by index, with scores ordered from high to low", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 1, &common_exec),
  CommandHolder("ZREVRANGEBYLEX", "<key> <max> <min> [LIMIT offset count]",
              "Return a range of members in a sorted set, by lexicographical range, ordered from higher to lower strings.", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 1, &common_exec),
  CommandHolder("ZREVRANGEBYSCORE", "<key> <max> <min> [WITHSCORES] [LIMIT offset count]",
              "Return a range of members in a sorted set, by score, with scores ordered from high to low", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 2, &common_exec),
  CommandHolder("ZREVRANK", "<key> <member>",
              "Determine the index of a member in a sorted set, with scores ordered from high to low", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("ZSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate sorted sets elements and associated scores", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 4, &common_exec),
  CommandHolder("ZSCORE", "<key> <member>",
              "Get the score associated with the given member in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0, &common_exec),
  CommandHolder("ZUNIONSTORE", "<destination> <numkeys> <key> [key ...] [WEIGHTS weight] [AGGREGATE SUM|MIN|MAX]",
              "Add multiple sorted sets and store the resulting sorted set in a new key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 3, &common_exec),

  CommandHolder("SENTINEL MASTERS", "-",
              "Show a list of monitored masters and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("SENTINEL MASTER", "<master name>",
              "Show the state and info of the specified master.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL SLAVES", "<master name>",
              "Show a list of slaves for this master, and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL SENTINELS", "<master name>",
              "Show a list of sentinel instances for this master, and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL GET-MASTER-ADDR-BY-NAME", "<master name>",
              "Return the ip and port number of the master with that name.\n"
              "If a failover is in progress or terminated successfully for this master "
              "it returns the address and port of the promoted slave.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL RESET", "<pattern>",
              "This command will reset all the masters with matching name.\n"
              "The pattern argument is a glob-style pattern.\n"
              "The reset process clears any previous state in a master (including a failover in progress), "
              "and removes every slave and sentinel already discovered and associated with the master.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL FAILOVER", "<master name>",
              "Force a failover as if the master was not reachable, "
              "and without asking for agreement to other Sentinels "
              "(however a new version of the configuration will be published so that the other "
              "Sentinels will update their configurations).", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL CKQUORUM", "<master name>",
              "Check if the current Sentinel configuration is able to reach the quorum needed to failover a master, "
              "and the majority needed to authorize the failover.\n"
              "This command should be used in monitoring systems to check if a Sentinel deployment is ok.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL FLUSHCONFIG", "-",
              "Force Sentinel to rewrite its configuration on disk, including the current Sentinel state.\n"
              "Normally Sentinel rewrites the configuration every time something changes in its state "
              "(in the context of the subset of the state which is persisted on disk across restart).\n"
              "However sometimes it is possible that the configuration file is lost because of operation errors, "
              "disk failures, package upgrade scripts or configuration managers.\n"
              "In those cases a way to to force Sentinel to rewrite the configuration file is handy.\n"
              "This command works even if the previous configuration file is completely missing.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0, &common_exec_off2),
  CommandHolder("SENTINEL MONITOR", "<name> <ip> <port> <quorum>",
              "This command tells the Sentinel to start monitoring a new master with the specified name, ip, port, and quorum.\n"
              "It is identical to the sentinel monitor configuration directive in sentinel.conf configuration file, "
              "with the difference that you can't use an hostname in as ip, but you need to provide an IPv4 or IPv6 address.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0, &common_exec_off2),
  CommandHolder("SENTINEL REMOVE", "<name>",
              "Used in order to remove the specified master: the master will no longer be monitored, "
              "and will totally be removed from the internal state of the Sentinel, "
              "so it will no longer listed by SENTINEL masters and so forth.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0, &common_exec_off2),
  CommandHolder("SENTINEL SET", "<name> <option> <value>",
              "The SET command is very similar to the CONFIG SET command of Redis, "
              "and is used in order to change configuration parameters of a specific master.\n"
              "Multiple option / value pairs can be specified (or none at all).\n"
              "All the configuration parameters that can be configured via "
              "sentinel.conf are also configurable using the SET command.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0, &common_exec_off2),

  CommandHolder("HELP", "[@group][command]",
              "help @<group> to get a list of commands in <group>"
              "help <command> for help on <command>",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 2, &help)
};

typedef redisContext NativeConnection;
struct RConfig
  : public Config{
  explicit RConfig(const Config& config, const SSHInfo& sinfo);
  RConfig();

  SSHInfo ssh_info;
};

common::Error createConnection(const RConfig& config, NativeConnection** context);
common::Error createConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error testConnection(ConnectionSettings* settings);

common::Error discoveryClusterConnection(ConnectionSettings* settings, std::vector<ServerDiscoveryClusterInfoSPtr>* infos);
common::Error discoverySentinelConnection(ConnectionSettings* settings, std::vector<ServerDiscoverySentinelInfoSPtr>* infos);

class IDBConnectionOwner {
public:
  virtual void currentDataBaseChanged(IDataBaseInfo* info) = 0;
};

class DBConnection
  : public core::DBConnection<NativeConnection, RConfig, REDIS>, public CommandHandler {
 public:
  typedef core::DBConnection<NativeConnection, RConfig, REDIS> base_class;
  explicit DBConnection(IDBConnectionOwner* observer);

  bool isAuthenticated() const;

  common::Error connect(const config_t& config);

  static const char* versionApi();

  common::Error latencyMode(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error slaveMode(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error getRDB(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error dbkcount(size_t* size) WARN_UNUSED_RESULT;
  common::Error findBigKeys(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error statMode(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error scanMode(FastoObject* out) WARN_UNUSED_RESULT;
  common::Error select(int num, IDataBaseInfo** info) WARN_UNUSED_RESULT;

  common::Error executeAsPipeline(std::vector<FastoObjectCommandIPtr> cmds) WARN_UNUSED_RESULT;

  common::Error common_exec(int argc, char** argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error common_exec_off2(int argc, char** argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error auth(const std::string& password) WARN_UNUSED_RESULT;
  common::Error help(int argc, char** argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error monitor(int argc, char** argv, FastoObject* out) WARN_UNUSED_RESULT;  // interrupt
  common::Error subscribe(int argc, char** argv, FastoObject* out) WARN_UNUSED_RESULT;  // interrupt

 private:
  common::Error sendSync(unsigned long long* payload) WARN_UNUSED_RESULT;
  common::Error sendScan(unsigned long long* it, redisReply** out) WARN_UNUSED_RESULT;
  common::Error getKeyTypes(redisReply* keys, int* types) WARN_UNUSED_RESULT;
  common::Error getKeySizes(redisReply* keys, int* types, unsigned long long* sizes) WARN_UNUSED_RESULT;

  common::Error cliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) WARN_UNUSED_RESULT;
  common::Error cliFormatReplyRaw(FastoObject* out, redisReply* r) WARN_UNUSED_RESULT;
  common::Error cliReadReply(FastoObject* out) WARN_UNUSED_RESULT;

  bool isAuth_;
  IDBConnectionOwner* const observer_;
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
