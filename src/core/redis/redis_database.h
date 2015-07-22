#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class RedisDatabase
            : public IDatabase
    {
        friend class RedisServer;
    private:
        RedisDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
