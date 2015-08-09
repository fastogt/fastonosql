#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class RedisServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        RedisServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
