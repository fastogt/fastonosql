#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class LeveldbServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        LeveldbServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
