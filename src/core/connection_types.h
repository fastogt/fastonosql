#pragma once

#include "common/convert2string.h"

namespace fastoredis
{
    enum connectionTypes
    {
        DBUNKNOWN = 0,
        REDIS,
        MEMCACHED,
        SSDB
    };

    enum serverTypes
    {
        MASTER,
        SLAVE
    };

    static const std::string connnectionType[] = { "Unknown", "Redis", "Memcached", "Ssdb" };

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

        /* Intaractive mode */
        IntaractiveMode
    };
}

namespace common
{
    std::string convertToString(fastoredis::connectionTypes t);
    std::string convertToString(fastoredis::serverTypes st);
    std::string convertToString(fastoredis::ConnectionMode t);
}
