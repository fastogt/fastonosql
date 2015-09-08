#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class LmdbServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        LmdbServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
