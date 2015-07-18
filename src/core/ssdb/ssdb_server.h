#pragma once

#include "core/iserver.h"

namespace fastoredis
{
    class SsdbServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabaseImpl(DataBaseInfoSPtr info);
        SsdbServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
