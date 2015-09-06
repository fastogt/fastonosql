#pragma once

#include "core/iserver.h"

namespace fastonosql
{
    class UnqliteServer
            : public IServer
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info);
        UnqliteServer(const IDriverSPtr& drv, bool isSuperServer);
    };
}
