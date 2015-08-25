#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class RocksdbServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        RocksdbServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
