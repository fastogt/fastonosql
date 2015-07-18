#include "core/redis/redis_server.h"

#include "core/redis/redis_database.h"

namespace fastoredis
{
    RedisServer::RedisServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv, isSuperServer)
    {

    }

    IDatabaseSPtr RedisServer::createDatabaseImpl(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new RedisDatabase(shared_from_this(), info));
    }
}
