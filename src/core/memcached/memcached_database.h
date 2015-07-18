#pragma once

#include "core/idatabase.h"

namespace fastoredis
{
    class MemcachedDatabase
            : public IDatabase
    {
        friend class MemcachedServer;
    private:
        MemcachedDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
