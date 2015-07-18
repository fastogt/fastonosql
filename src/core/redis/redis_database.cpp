#include "core/redis/redis_database.h"

#include "core/iserver.h"

namespace fastoredis
{
    RedisDatabase::RedisDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == REDIS);
        DCHECK(info->type() == REDIS);
    }
}
