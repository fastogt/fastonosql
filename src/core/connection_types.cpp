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
const char* kConnnectionType[] = {"Redis",   "Memcached", "SSDB",      "LevelDB",  "RocksDB",
                                  "UnQLite", "LMDB",      "UpscaleDB", "ForestDB", "Pika"};
const std::string kConnnectionMode[] = {"Interactive mode"};
const std::string kServerTypes[] = {"Master", "Slave"};
const std::string kServerState[] = {"Up", "Down"};
const std::string kServerModes[] = {"Standalone", "Sentinel", "Cluster"};
}  // namespace

namespace fastonosql {
namespace core {

const std::vector<connectionTypes> g_compiled_types = {
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
    FORESTDB,
#endif
#ifdef BUILD_WITH_PIKA
    PIKA
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
  return kConnnectionType[t];
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

  for (size_t i = 0; i < SIZEOFMASS(kConnnectionType); ++i) {
    if (from == kConnnectionType[i]) {
      *out = static_cast<fastonosql::core::connectionTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::ServerTypes* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(kServerTypes); ++i) {
    if (from == kServerTypes[i]) {
      *out = static_cast<fastonosql::core::ServerTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::ServerState* out) {
  for (size_t i = 0; i < SIZEOFMASS(kServerState); ++i) {
    if (from == kServerState[i]) {
      *out = static_cast<fastonosql::core::ServerState>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

std::string ConvertToString(fastonosql::core::ServerTypes st) {
  return kServerTypes[st];
}

std::string ConvertToString(fastonosql::core::ServerState st) {
  return kServerState[st];
}

std::string ConvertToString(fastonosql::core::ServerMode md) {
  return kServerModes[md];
}

std::string ConvertToString(fastonosql::core::ConnectionMode t) {
  return kConnnectionMode[t];
}

}  // namespace common
