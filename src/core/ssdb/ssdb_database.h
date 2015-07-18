#pragma once

#include "core/idatabase.h"

namespace fastoredis
{
    class SsdbDatabase
            : public IDatabase
    {
        friend class SsdbServer;
    private:
        SsdbDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
