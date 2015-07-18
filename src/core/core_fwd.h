#pragma once

#include "core/types.h"

namespace fastoredis
{
    typedef common::shared_ptr<DataBaseInfo> DataBaseInfoSPtr;

    class IDatabase;
    typedef common::shared_ptr<IDatabase> IDatabaseSPtr;

    class IServer;
    typedef common::shared_ptr<IServer> IServerSPtr;

    class IDriver;
    typedef common::shared_ptr<IDriver> IDriverSPtr;

    class ICluster;
    typedef common::shared_ptr<ICluster> IClusterSPtr;
}
