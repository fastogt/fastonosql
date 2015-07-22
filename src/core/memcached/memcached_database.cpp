#include "core/memcached/memcached_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    MemcachedDatabase::MemcachedDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == MEMCACHED);
        DCHECK(info->type() == MEMCACHED);
    }
}
