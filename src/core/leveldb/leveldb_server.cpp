#include "core/leveldb/leveldb_server.h"

#include "core/leveldb/leveldb_database.h"

namespace fastonosql
{
    LeveldbServer::LeveldbServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv,isSuperServer)
    {

    }

    IDatabaseSPtr LeveldbServer::createDatabaseImpl(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new LeveldbDatabase(shared_from_this(), info));
    }
}
