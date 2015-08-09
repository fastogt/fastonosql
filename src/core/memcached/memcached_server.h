#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class MemcachedServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        MemcachedServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
