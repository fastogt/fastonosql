#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class SsdbServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        SsdbServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
