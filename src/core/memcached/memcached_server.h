#pragma once

#include "core/iserver.h"

namespace fastoredis
{
    class MemcachedServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabaseImpl(DataBaseInfoSPtr info);
        MemcachedServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
