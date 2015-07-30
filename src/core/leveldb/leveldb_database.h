#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class LeveldbDatabase
            : public IDatabase
    {
        friend class LeveldbServer;
    private:
        LeveldbDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
