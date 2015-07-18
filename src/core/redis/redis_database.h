#pragma once

#include "core/idatabase.h"

namespace fastoredis
{
    class RedisDatabase
            : public IDatabase
    {
        friend class RedisServer;
    private:
        RedisDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
