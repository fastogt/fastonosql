#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class LmdbDatabase
            : public IDatabase
    {
        friend class LmdbServer;
    private:
        LmdbDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
