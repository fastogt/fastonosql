#pragma once

#include "common/convert2string.h"

namespace fastonosql
{
    enum connectionTypes
    {
        DBUNKNOWN = 0,
        REDIS,
        MEMCACHED,
        SSDB,
        LEVELDB,
        ROCKSDB,
        UNQLITE,
        LMDB
    };

    enum serverTypes
    {
        MASTER,
        SLAVE
    };

    static const std::string connnectionType[] =
    {
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

    enum ConnectionMode
    {
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
}

namespace common
{
    std::string convertToString(fastonosql::connectionTypes t);
    std::string convertToString(fastonosql::serverTypes st);
    std::string convertToString(fastonosql::ConnectionMode t);
}
