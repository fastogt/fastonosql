#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class MemcachedDatabase
            : public IDatabase
    {
        friend class MemcachedServer;
    private:
        MemcachedDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
