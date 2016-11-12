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

#include <vector>
#include <string>  // for string

namespace fastonosql {
namespace core {

enum connectionTypes { REDIS = 0, MEMCACHED, SSDB, LEVELDB, ROCKSDB, UNQLITE, LMDB, UPSCALEDB };
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
    UPSCALEDB
#endif
};

enum ConnectionMode {
  /* Latency mode */
  LatencyMode,
  /* Slave mode */
  SlaveMode,
  /* Get RDB mode. */
  GetRDBMode,
  /* Pipe mode */
  PipeMode,
  /* Find big keys */
  FindBigKeysMode,
  /* Stat mode */
  StatMode,
  /* Scan mode */
  ScanMode,
  /* Interactive mode */
  InteractiveMode
};

bool IsRemoteType(connectionTypes type);
bool IsCanSSHConnection(connectionTypes type);
const char* CommandLineHelpText(connectionTypes type);

}  // namespace core
}  // namespace fastonosql

namespace common {
std::string ConvertToString(fastonosql::core::connectionTypes t);
std::string ConvertToString(fastonosql::core::serverTypes st);
std::string ConvertToString(fastonosql::core::serverState st);
std::string ConvertToString(fastonosql::core::serverMode md);
std::string ConvertToString(fastonosql::core::ConnectionMode t);
}
