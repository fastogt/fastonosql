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

#include <string>  // for string

namespace fastonosql {
namespace core {

enum connectionTypes {
  REDIS = 0,
  MEMCACHED,
  SSDB,
  LEVELDB,
  ROCKSDB,
  UNQLITE,
  LMDB,
  UPSCALEDB,
  FORESTDB
};  // supported types
enum serverTypes { MASTER = 0, SLAVE };
enum serverState { SUP = 0, SDOWN };
enum serverConnectState { SCONNECTED = 0, SDISCONNECTED };
enum serverMode { STANDALONE = 0, SENTINEL, CLUSTER };

static const connectionTypes compiled_types[] = {
#ifdef BUILD_WITH_REDIS
    REDIS,
#endif
#ifdef BUILD_WITH_MEMCACHED
    MEMCACHED,
#endif
#ifdef BUILD_WITH_SSDB
    SSDB,
#endif
#ifdef BUILD_WITH_LEVELDB
    LEVELDB,
#endif
#ifdef BUILD_WITH_ROCKSDB
    ROCKSDB,
#endif
#ifdef BUILD_WITH_UNQLITE
    UNQLITE,
#endif
#ifdef BUILD_WITH_LMDB
    LMDB,
#endif
#ifdef BUILD_WITH_UPSCALEDB
    UPSCALEDB,
#endif
#ifdef BUILD_WITH_FORESTDB
    FORESTDB
#endif
};

enum ConnectionMode { InteractiveMode };

bool IsRemoteType(connectionTypes type);
bool IsSupportTTLKeys(connectionTypes type);
bool IsLocalType(connectionTypes type);
bool IsCanSSHConnection(connectionTypes type);
const char* ConnectionTypeToString(connectionTypes t);

template <connectionTypes ContType>
struct ConnectionTraits {
  static const char* GetDBName() { return ConnectionTypeToString(ContType); }
  static const char* BasedOn();
  static const char* VersionApi();
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
