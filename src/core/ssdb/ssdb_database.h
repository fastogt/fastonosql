#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class SsdbDatabase
            : public IDatabase
    {
        friend class SsdbServer;
    private:
        SsdbDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
