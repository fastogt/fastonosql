#pragma once

#include "core/idatabase.h"

namespace fastonosql
{
    class UnqliteDatabase
            : public IDatabase
    {
        friend class UnqliteServer;
    private:
        UnqliteDatabase(IServerSPtr server, DataBaseInfoSPtr info);
    };
}
