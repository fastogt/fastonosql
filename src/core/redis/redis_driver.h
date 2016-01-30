/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "core/idriver.h"

#include "core/redis/redis_settings.h"

namespace fastonosql {

const CommandInfo redisCommands[] = {
  CommandInfo("APPEND", "<key> <value>",
              "Append a value to a key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("AUTH", "<password>",
              "Authenticate to the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("BGREWRITEAOF", "-",
              "Asynchronously rewrite the append-only file", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("BGSAVE", "-",
              "Asynchronously save the dataset to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("BITCOUNT","<key> [start] [end]",
              "Count set bits in a string", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 2),
  CommandInfo("BITOP", "<operation> <destkey> <key> [key ...]",
              "Perform bitwise operations between strings", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 2),
  CommandInfo("BITPOS", "<key> <bit> [start] [end]",
              "Find first bit set or clear in a string", PROJECT_VERSION_GENERATE(2,8,7), UNDEFINED_EXAMPLE_STR, 2, 2),
  CommandInfo("BLPOP", "<key> [key ...] timeout",
              "Remove and get the first element in a list, or block until one is available", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("BRPOP", "<key> [key ...] timeout",
              "Remove and get the last element in a list, or block until one is available", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("BRPOPLPUSH", "source destination timeout",
              "Pop a value from a list, push it to another list and return it; or block until one is available", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLIENT GETNAME", "-",
              "Get the current connection name", PROJECT_VERSION_GENERATE(2,6,9), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLIENT KILL", "<ip:port>",
              "Kill the connection of a client", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("CLIENT LIST", "-",
              "Get the list of client connections", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLIENT PAUSE", "<timeout>",
              "Stop processing commands from clients for some time", PROJECT_VERSION_GENERATE(2,9,50), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("CLIENT SETNAME", "<connection-name>",
              "Set the current connection name", PROJECT_VERSION_GENERATE(2,6,9), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("CONFIG GET", "<parameter>",
              "Get the value of a configuration parameter", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("CONFIG RESETSTAT", "-",
              "Reset the stats returned by INFO", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CONFIG REWRITE", "-",
              "Rewrite the configuration file with the in memory configuration", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CONFIG SET", "<parameter> <value>",
              "Set a configuration parameter to the given value", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("DBSIZE", "-",
              "Return the number of keys in the selected database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("DEBUG OBJECT", "<key>",
              "Get debugging information about a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("DEBUG SEGFAULT", "-",
              "Make the server crash", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("DECR", "<key>",
              "Decrement the integer value of a key by one", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("DECRBY", "<key> <decrement>",
              "Decrement the integer value of a key by the given number", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("DEL", "<key> [key ...]",
              "Delete a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("DISCARD", "-",
              "Discard all commands issued after MULTI", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("DUMP", "<key>",
              "Return a serialized version of the value stored at the specified key.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("ECHO", "<message>",
              "Echo the given string", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("EVAL", "script numkeys <key> [key ...] <arg> [arg ...]",
              "Execute a Lua script server side", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("EVALSHA", "sha1 numkeys <key> [key ...] <arg> [arg ...]",
              "Execute a Lua script server side", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("EXEC", "-",
              "Execute all commands issued after MULTI", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("EXISTS",
              "<key>", "Determine if a key exists", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("EXPIRE", "<key> <seconds>",
              "Set a key's time to live in seconds", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("EXPIREAT", "<key> <timestamp>",
              "Set the expiration for a key as a UNIX timestamp", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("FLUSHALL", "-",
              "Remove all keys from all databases", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("FLUSHDB", "-",
              "Remove all keys from the current database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("GET", "<key>",
              "Get the value of a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("GETBIT", "<key> <offset>",
              "Returns the bit value at offset in the string value stored at key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("GETRANGE", "<key> <start> <end>",
              "Get a substring of the string stored at a key", PROJECT_VERSION_GENERATE(2,4,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("GETSET", "<key> <value>",
              "Set the string value of a key and return its old value", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("HDEL", "<key> <field> [field ...]",
              "Delete one or more hash fields", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("HEXISTS", "<key> <field>",
              "Determine if a hash field exists", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("HGET", "<key> <field>",
              "Get the value of a hash field", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("HGETALL", "<key>",
              "Get all the fields and values in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("HINCRBY", "<key> <field> <increment>",
              "Increment the integer value of a hash field by the given number", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("HINCRBYFLOAT", "<key> <field> <increment>",
              "Increment the float value of a hash field by the given amount", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("HKEYS", "<key>",
              "Get all the fields in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("HLEN", "<key>",
              "Get the number of fields in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("HMGET", "<key> <field> [field ...]",
              "Get the values of all the given hash fields", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("HMSET", "<key> <field> <value> [field value ...]",
              "Set multiple hash fields to multiple values", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("HSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate hash fields and associated values", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 2),
  CommandInfo("HSET", "<key> <field> <value>",
              "Set the string value of a hash field", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("HSETNX", "<key> <field> <value>",
              "Set the value of a hash field, only if the field does not exist", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("HVALS", "<key>",
              "Get all the values in a hash", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("INCR", "<key>",
              "Increment the integer value of a key by one", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("INCRBY", "<key> <increment>",
              "Increment the integer value of a key by the given amount", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("INCRBYFLOAT", "<key> <increment>",
              "Increment the float value of a key by the given amount", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("INFO", "[section]",
              "Get information and statistics about the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 1),
  CommandInfo("INTERRUPT", "-",
              "Command execution interrupt",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("KEYS", "<pattern>",
              "Find all keys matching the given pattern", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("LASTSAVE", "-",
              "Get the UNIX time stamp of the last successful save to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("LINDEX", "<key> <index>",
              "Get an element from a list by its index", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("LINSERT", "<key> <BEFORE|AFTER> <pivot value>",
              "Insert an element before or after another element in a list", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("LLEN", "<key>",
              "Get the length of a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("LPOP", "<key>",
              "Remove and get the first element in a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("LPUSH", "<key> <value> [value ...]",
              "Prepend one or multiple values to a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("LPUSHX", "<key> <value>",
              "Prepend a value to a list, only if the list exists", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("LRANGE", "<key> <start> <stop>",
              "Get a range of elements from a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("LREM", "<key> <count> <value>",
              "Remove elements from a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("LSET", "<key> <index> <value>",
              "Set the value of an element in a list by its index", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("LTRIM", "<key> <start> <stop>",
              "Trim a list to the specified range", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("MGET", "<key> [key ...]",
              "Get the values of all the given keys", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("MIGRATE", "<host> <port> <key> <destination-db> <timeout> [COPY] [REPLACE]",
              "Atomically transfer a key from a Redis instance to another one.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 5, 2),
  CommandInfo("MONITOR", "-",
              "Listen for all requests received by the server in real time", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("MOVE", "<key> <db>",
              "Move a key to another database", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("MSET", "<key> <value> [key value ...]",
              "Set multiple keys to multiple values", PROJECT_VERSION_GENERATE(1,0,1), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("MSETNX", "<key> <value> [key value ...]",
              "Set multiple keys to multiple values, only if none of the keys exist", PROJECT_VERSION_GENERATE(1,0,1), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("MULTI", "-",
              "Mark the start of a transaction block", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("OBJECT", "<subcommand> [arguments [arguments ...]]",
              "Inspect the internals of Redis objects", PROJECT_VERSION_GENERATE(2,2,3), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PERSIST", "<key>",
              "Remove the expiration from a key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PEXPIRE", "<key> <milliseconds>",
              "Set a key's time to live in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PEXPIREAT", "<key> <milliseconds-timestamp>",
              "Set the expiration for a key as a UNIX timestamp specified in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PFADD", "<key> <element> [element ...]",
              "Adds the specified elements to the specified HyperLogLog.", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PFCOUNT", "<key> [key ...]",
              "Return the approximated cardinality of the set(s) observed by the HyperLogLog at key(s).", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PFMERGE", "<destkey> <sourcekey> [sourcekey ...]",
              "Merge N different HyperLogLogs into a single one.", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PING", "-",
              "Ping the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("PSETEX", "<key> <milliseconds> <value>",
              "Set the value and expiration in milliseconds of a key", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("PSUBSCRIBE", "<pattern> [pattern ...]",
              "Listen for messages published to channels matching the given patterns", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PTTL", "<key>",
              "Get the time to live for a key in milliseconds", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PUBLISH", "<channel> <message>",
              "Post a message to a channel", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PUBSUB", "<subcommand> [argument [argument ...]]",
              "Inspect the state of the Pub/Sub subsystem", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("PUNSUBSCRIBE", "[pattern [pattern ...]]",
              "Stop listening for messages posted to channels matching the given patterns", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 1),
  CommandInfo("QUIT", "-",
              "Close the connection", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("RANDOMKEY", "-",
              "Return a random key from the keyspace", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("RENAME", "<key> <newkey>",
              "Rename a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("RENAMENX", "<key> <newkey>",
              "Rename a key, only if the new key does not exist", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("RESTORE", "<key> <ttl> <serialized-value>",
              "Create a key using the provided serialized value, previously obtained using DUMP.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("RPOP", "<key>",
              "Remove and get the last element in a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("RPOPLPUSH", "<source> <destination>",
              "Remove the last element in a list, append it to another list and return it", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("RPUSH", "<key> <value> [value ...]",
              "Append one or multiple values to a list", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("RPUSHX", "<key> <value>",
              "Append a value to a list, only if the list exists", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SADD", "<key> <member> [member ...]",
              "Add one or more members to a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SAVE", "-",
              "Synchronously save the dataset to disk", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("SCAN", "<cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate the keys space", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 1, 2),
  CommandInfo("SCARD", "<key>",
              "Get the number of members in a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SCRIPT EXISTS", "script [script ...]",
              "Check existence of scripts in the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SCRIPT FLUSH", "-",
              "Remove all the scripts from the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("SCRIPT KILL", "-",
              "Kill the script currently in execution.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("SCRIPT LOAD", "<script>",
              "Load the specified Lua script into the script cache.", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SDIFF", "<key> [key ...]",
              "Subtract multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SDIFFSTORE", "<destination> <key> [key ...]",
              "Subtract multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SELECT", "<index>",
              "Change the selected database for the current connection", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SET", "<key> <value> [EX seconds] [PX milliseconds] [NX|XX]",
              "Set the string value of a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 3),
  CommandInfo("SETBIT", "<key> <offset> <value>",
              "Sets or clears the bit at offset in the string value stored at key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("SETEX", "<key> <seconds> <value>",
              "Set the value and expiration of a key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("SETNX", "<key> <value>",
              "Set the value of a key, only if the key does not exist", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SETRANGE", "<key> <offset> <value>",
              "Overwrite part of a string at key starting at the specified offset", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("SHUTDOWN", "[NOSAVE] [SAVE]",
              "Synchronously save the dataset to disk and then shut down the server", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 1),
  CommandInfo("SINTER", "<key> [key ...]",
              "Intersect multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SINTERSTORE", "<destination> <key> [key ...]",
              "Intersect multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SISMEMBER", "<key> <member>",
              "Determine if a given value is a member of a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SLAVEOF", "<host> <port>",
              "Make the server a slave of another instance, or promote it as master", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SLOWLOG", "<subcommand> [argument]",
              "Manages the Redis slow queries log", PROJECT_VERSION_GENERATE(2,2,12), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SMEMBERS", "<key>",
              "Get all the members in a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SMOVE", "<source> <destination> <member>",
              "Move a member from one set to another", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("SORT", "<key> [BY pattern] [LIMIT offset count] [GET pattern [GET pattern ...]] [ASC|DESC] [ALPHA] [STORE destination]",
              "Sort the elements in a list, set or sorted set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 6),
  CommandInfo("SPOP", "<key> [count]",
              "Remove and return one or multiple random members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 1),
  CommandInfo("SRANDMEMBER",
              "<key> [count]", "Get one or multiple random members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 1),
  CommandInfo("SREM", "<key> <member> [member ...]",
              "Remove one or more members from a set", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 1),
  CommandInfo("SSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate Set elements", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 2),
  CommandInfo("STRLEN", "<key>",
              "Get the length of the value stored in a key", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SUBSCRIBE", "<channel> [channel ...]",
              "Listen for messages published to the given channels", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SUNION", "<key> [key ...]",
              "Add multiple sets", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SUNIONSTORE", "<destination> <key> [key ...]",
              "Add multiple sets and store the resulting set in a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("SYNC", "-",
              "Internal command used for replication", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("TIME", "-",
              "Return the current server time", PROJECT_VERSION_GENERATE(2,6,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("TTL", "<key>",
              "Get the time to live for a key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("TYPE", "<key>",
              "Determine the type stored at key", PROJECT_VERSION_GENERATE(1,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("UNSUBSCRIBE", "[channel [channel ...]]",
              "Stop listening for messages posted to the given channels", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("UNWATCH", "-",
              "Forget about all watched keys", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("WATCH", "key [key ...]",
              "Watch the given keys to determine execution of the MULTI/EXEC block", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("ZADD", "<key> <score> <member> [score member ...]",
              "Add one or more members to a sorted set, or update its score if it already exists", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZCARD", "<key>",
              "Get the number of members in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("ZCOUNT", "<key> <min> <max>",
              "Count the members in a sorted set with scores within the given values", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZINCRBY", "<key> <increment> <member>",
              "Increment the score of a member in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZINTERSTORE", "<destination> <numkeys> <key> [key ...] [WEIGHTS weight] [AGGREGATE SUM|MIN|MAX]",
              "Intersect multiple sorted sets and store the resulting sorted set in a new key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 2),
  CommandInfo("ZLEXCOUNT", "<key> <min> <max>",
              "Count the number of members in a sorted set between a given lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZRANGE", "<key> <start> <stop> [WITHSCORES]",
              "Return a range of members in a sorted set, by index", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 1),
  CommandInfo("ZRANGEBYLEX", "<key> <min> <max> [LIMIT offset count]",
              "Return a range of members in a sorted set, by lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 3),
  CommandInfo("ZRANGEBYSCORE", "<key> <min> <max> [WITHSCORES] [LIMIT offset count]",
              "Return a range of members in a sorted set, by score", PROJECT_VERSION_GENERATE(1,0,5), UNDEFINED_EXAMPLE_STR, 3, 3),
  CommandInfo("ZRANK", "<key> <member>",
              "Determine the index of a member in a sorted set", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("ZREM", "<key> <member> [member ...]",
              "Remove one or more members from a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("ZREMRANGEBYLEX", "<key> <min> <max>",
              "Remove all members in a sorted set between the given lexicographical range", PROJECT_VERSION_GENERATE(2,8,9), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZREMRANGEBYRANK", "<key> <start> <stop>",
              "Remove all members in a sorted set within the given indexes", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZREMRANGEBYSCORE", "<key> <min> <max>",
              "Remove all members in a sorted set within the given scores", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("ZREVRANGE", "<key> <start> <stop> [WITHSCORES]",
              "Return a range of members in a sorted set, by index, with scores ordered from high to low", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 3, 1),
  CommandInfo("ZREVRANGEBYSCORE", "<key> <max> <min> [WITHSCORES] [LIMIT offset count]",
              "Return a range of members in a sorted set, by score, with scores ordered from high to low", PROJECT_VERSION_GENERATE(2,2,0), UNDEFINED_EXAMPLE_STR, 3, 2),
  CommandInfo("ZREVRANK", "<key> <member>",
              "Determine the index of a member in a sorted set, with scores ordered from high to low", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("ZSCAN", "<key> <cursor> [MATCH pattern] [COUNT count]",
              "Incrementally iterate sorted sets elements and associated scores", PROJECT_VERSION_GENERATE(2,8,0), UNDEFINED_EXAMPLE_STR, 2, 4),
  CommandInfo("ZSCORE", "<key> <member>",
              "Get the score associated with the given member in a sorted set", PROJECT_VERSION_GENERATE(1,2,0), UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("ZUNIONSTORE", "<destination> <numkeys> <key> [key ...] [WEIGHTS weight] [AGGREGATE SUM|MIN|MAX]",
              "Add multiple sorted sets and store the resulting sorted set in a new key", PROJECT_VERSION_GENERATE(2,0,0), UNDEFINED_EXAMPLE_STR, 3, 3),

  CommandInfo("SENTINEL MASTERS", "-",
              "Show a list of monitored masters and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("SENTINEL MASTER", "<master name>",
              "Show the state and info of the specified master.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL SLAVES", "<master name>",
              "Show a list of slaves for this master, and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL SENTINELS", "<master name>",
              "Show a list of sentinel instances for this master, and their state.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL GET-MASTER-ADDR-BY-NAME", "<master name>",
              "Return the ip and port number of the master with that name.\n"
              "If a failover is in progress or terminated successfully for this master "
              "it returns the address and port of the promoted slave.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL RESET", "<pattern>",
              "This command will reset all the masters with matching name.\n"
              "The pattern argument is a glob-style pattern.\n"
              "The reset process clears any previous state in a master (including a failover in progress), "
              "and removes every slave and sentinel already discovered and associated with the master.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL FAILOVER", "<master name>",
              "Force a failover as if the master was not reachable, "
              "and without asking for agreement to other Sentinels "
              "(however a new version of the configuration will be published so that the other "
              "Sentinels will update their configurations).", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL CKQUORUM", "<master name>",
              "Check if the current Sentinel configuration is able to reach the quorum needed to failover a master, "
              "and the majority needed to authorize the failover.\n"
              "This command should be used in monitoring systems to check if a Sentinel deployment is ok.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL FLUSHCONFIG", "-",
              "Force Sentinel to rewrite its configuration on disk, including the current Sentinel state.\n"
              "Normally Sentinel rewrites the configuration every time something changes in its state "
              "(in the context of the subset of the state which is persisted on disk across restart).\n"
              "However sometimes it is possible that the configuration file is lost because of operation errors, "
              "disk failures, package upgrade scripts or configuration managers.\n"
              "In those cases a way to to force Sentinel to rewrite the configuration file is handy.\n"
              "This command works even if the previous configuration file is completely missing.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("SENTINEL MONITOR", "<name> <ip> <port> <quorum>",
              "This command tells the Sentinel to start monitoring a new master with the specified name, ip, port, and quorum.\n"
              "It is identical to the sentinel monitor configuration directive in sentinel.conf configuration file, "
              "with the difference that you can't use an hostname in as ip, but you need to provide an IPv4 or IPv6 address.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("SENTINEL REMOVE", "<name>",
              "Used in order to remove the specified master: the master will no longer be monitored, "
              "and will totally be removed from the internal state of the Sentinel, "
              "so it will no longer listed by SENTINEL masters and so forth.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("SENTINEL SET", "<name> <option> <value>",
              "The SET command is very similar to the CONFIG SET command of Redis, "
              "and is used in order to change configuration parameters of a specific master.\n"
              "Multiple option / value pairs can be specified (or none at all).\n"
              "All the configuration parameters that can be configured via "
              "sentinel.conf are also configurable using the SET command.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("CLUSTER SLOTS", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLUSTER NODES", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLUSTER SLAVES", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLUSTER ADDSLOTS", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLUSTER REPLICATE", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("CLUSTER SETSLOT", UNDEFINED_STR_IN_PROGRESS,
              UNDEFINED_STR_IN_PROGRESS, UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0)
};

common::Error testConnection(RedisConnectionSettings* settings);
common::Error discoveryConnection(RedisConnectionSettings* settings, std::vector<ServerDiscoveryInfoSPtr>& infos);

class RedisDriver
      : public IDriver
{
  Q_OBJECT
public:
  explicit RedisDriver(IConnectionSettingsBaseSPtr settings);
  virtual ~RedisDriver();

  virtual bool isConnected() const;
  virtual bool isAuthenticated() const;
  common::net::hostAndPort address() const;
  virtual std::string outputDelemitr() const;

  static const char* versionApi();

private:
  virtual void initImpl();
  virtual void clearImpl();

  virtual common::Error executeImpl(FastoObject* out, int argc, char **argv);

  virtual common::Error serverInfo(ServerInfo** info);
  virtual common::Error serverDiscoveryInfo(ServerInfo** sinfo, ServerDiscoveryInfo** dinfo, DataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(DataBaseInfo** info);

  virtual void handleConnectEvent(events::ConnectRequestEvent* ev);
  virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev);
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev);
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
  virtual void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);
  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void handleBackupEvent(events::BackupRequestEvent* ev);
  virtual void handleExportEvent(events::ExportRequestEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

  virtual common::Error commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandChangeTTLImpl(CommandChangeTTL* command, std::string& cmdstring) const WARN_UNUSED_RESULT;

  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev);

  ServerInfoSPtr makeServerInfoFromString(const std::string& val);

  struct pimpl;
  pimpl* const impl_;

  common::Error interacteveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error latencyMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error slaveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error getRDBMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error findBigKeysMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error statMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error scanMode(events::ProcessConfigArgsRequestEvent* ev);
};

}
