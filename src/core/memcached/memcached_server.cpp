#include "core/memcached/memcached_server.h"

#include "core/memcached/memcached_database.h"

namespace fastonosql
{
    MemcachedServer::MemcachedServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv, isSuperServer)
    {

    }

    IDatabaseSPtr MemcachedServer::createDatabase(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new MemcachedDatabase(shared_from_this(), info));
    }
}
