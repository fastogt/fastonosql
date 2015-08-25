#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class RocksdbDatabase
            : public IDatabase
    {
        friend class RocksdbServer;
    private:
        RocksdbDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
