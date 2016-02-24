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

#include <string>

#include "common/convert2string.h"

namespace fastonosql {

  enum connectionTypes {
    DBUNKNOWN = 0,
    REDIS,
    MEMCACHED,
    SSDB,
    LEVELDB,
    ROCKSDB,
    UNQLITE,
    LMDB
  };

  enum serverTypes {
    MASTER,
    SLAVE
  };

static const std::string connnectionType[] = {
  "Unknown",
#ifdef BUILD_WITH_REDIS
  "Redis",
#endif
#ifdef BUILD_WITH_MEMCACHED
  "Memcached",
#endif
#ifdef BUILD_WITH_SSDB
  "Ssdb",
#endif
#ifdef BUILD_WITH_LEVELDB
  "Leveldb",
#endif
#ifdef BUILD_WITH_ROCKSDB
  "Rocksdb",
#endif
#ifdef BUILD_WITH_UNQLITE
  "Unqlite",
#endif
#ifdef BUILD_WITH_LMDB
  "Lmdb"
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

}  // namespace fastonosql

namespace common {
  std::string convertToString(fastonosql::connectionTypes t);
  std::string convertToString(fastonosql::serverTypes st);
  std::string convertToString(fastonosql::ConnectionMode t);
}
