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

#include "core/connection_types.h"

#include <common/macros.h>  // for NOTREACHED, SIZEOFMASS

namespace {
const char* connnectionType[] = {"Redis",   "Memcached", "SSDB",      "LevelDB", "RocksDB",
                                 "UnQLite", "LMDB",      "UpscaleDB", "ForestDB"};
const std::string connnectionMode[] = {"Interactive mode"};
const std::string serverTypes[] = {"Master", "Slave"};
const std::string serverState[] = {"Up", "Down"};
const std::string serverModes[] = {"Standalone", "Sentinel", "Cluster"};
}  // namespace

namespace fastonosql {
namespace core {

const std::vector<connectionTypes> g_compiled_types = {
#ifdef BUILD_WITH_REDIS
    REDIS,
#endif
#ifdef BUILD_WITH_PIKA
    PIKA,
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

bool IsRedisCompatible(connectionTypes type) {
  return type == REDIS || type == PIKA;
}

bool IsRemoteType(connectionTypes type) {
  return type == REDIS || type == PIKA || type == MEMCACHED || type == SSDB;
}

bool IsSupportTTLKeys(connectionTypes type) {
  return type == REDIS || type == PIKA || type == MEMCACHED || type == SSDB;
}

bool IsLocalType(connectionTypes type) {
  return type == ROCKSDB || type == LEVELDB || type == LMDB || type == UPSCALEDB || type == UNQLITE || type == FORESTDB;
}

bool IsCanSSHConnection(connectionTypes type) {
  return IsRedisCompatible(type);
}

bool IsCanCreateDatabase(connectionTypes type) {
  return type == LMDB || type == FORESTDB || type == ROCKSDB;
}

bool IsCanRemoveDatabase(connectionTypes type) {
  return type == LMDB || type == FORESTDB || type == ROCKSDB;
}

const char* ConnectionTypeToString(connectionTypes t) {
  return connnectionType[t];
}

}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::core::connectionTypes t) {
  return fastonosql::core::ConnectionTypeToString(t);
}

bool ConvertFromString(const std::string& from, fastonosql::core::connectionTypes* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(connnectionType); ++i) {
    if (from == connnectionType[i]) {
      *out = static_cast<fastonosql::core::connectionTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::serverTypes* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(serverTypes); ++i) {
    if (from == serverTypes[i]) {
      *out = static_cast<fastonosql::core::serverTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::serverState* out) {
  for (size_t i = 0; i < SIZEOFMASS(serverState); ++i) {
    if (from == serverState[i]) {
      *out = static_cast<fastonosql::core::serverState>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

std::string ConvertToString(fastonosql::core::serverTypes st) {
  return serverTypes[st];
}

std::string ConvertToString(fastonosql::core::serverState st) {
  return serverState[st];
}

std::string ConvertToString(fastonosql::core::serverMode md) {
  return serverModes[md];
}

std::string ConvertToString(fastonosql::core::ConnectionMode t) {
  return connnectionMode[t];
}

}  // namespace common
