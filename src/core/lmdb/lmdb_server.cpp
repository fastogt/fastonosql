#include "core/lmdb/lmdb_server.h"

#include "core/lmdb/lmdb_database.h"

namespace fastonosql
{
    LmdbServer::LmdbServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv, isSuperServer)
    {

    }

    IDatabaseSPtr LmdbServer::createDatabase(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new LmdbDatabase(shared_from_this(), info));
    }
}
