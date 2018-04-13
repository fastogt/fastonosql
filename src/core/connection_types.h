/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <string>  // for string
#include <vector>

#define ALL_COMMANDS "*"
#define ALL_KEYS_PATTERNS "*"
#define ALL_PUBSUB_CHANNELS "*"
#define NO_KEYS_LIMIT INT32_MAX

namespace fastonosql {
namespace core {

typedef uint32_t keys_limit_t;
typedef keys_limit_t cursor_t;

enum connectionTypes {
  REDIS = 0,
  MEMCACHED,
  SSDB,
  LEVELDB,
  ROCKSDB,
  UNQLITE,
  LMDB,
  UPSCALEDB,
  FORESTDB,
  PIKA
};  // supported types
enum serverTypes { MASTER = 0, SLAVE };
enum serverState { SUP = 0, SDOWN };
enum serverConnectState { SCONNECTED = 0, SDISCONNECTED };
enum serverMode { STANDALONE = 0, SENTINEL, CLUSTER };

extern const std::vector<connectionTypes> g_compiled_types;

enum ConnectionMode { InteractiveMode };

bool IsRedisCompatible(connectionTypes type);
bool IsRemoteType(connectionTypes type);
bool IsSupportTTLKeys(connectionTypes type);
bool IsLocalType(connectionTypes type);
bool IsCanSSHConnection(connectionTypes type);
bool IsCanCreateDatabase(connectionTypes type);
bool IsCanRemoveDatabase(connectionTypes type);
const char* ConnectionTypeToString(connectionTypes t);

template <connectionTypes conection_type>
struct ConnectionTraits {
  static const char* GetDBName() { return ConnectionTypeToString(conection_type); }
  static const char* GetBasedOn();
  static const char* GetVersionApi();
};

}  // namespace core
}  // namespace fastonosql

namespace common {
std::string ConvertToString(fastonosql::core::connectionTypes t);
bool ConvertFromString(const std::string& from, fastonosql::core::connectionTypes* out);

std::string ConvertToString(fastonosql::core::serverTypes st);
bool ConvertFromString(const std::string& from, fastonosql::core::serverTypes* out);

std::string ConvertToString(fastonosql::core::serverState st);
bool ConvertFromString(const std::string& from, fastonosql::core::serverState* out);

std::string ConvertToString(fastonosql::core::serverMode md);
std::string ConvertToString(fastonosql::core::ConnectionMode t);
}  // namespace common
