#include "core/rocksdb/rocksdb_server.h"

#include "core/rocksdb/rocksdb_database.h"

namespace fastonosql
{
    RocksdbServer::RocksdbServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv, isSuperServer)
    {

    }

    IDatabaseSPtr RocksdbServer::createDatabase(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new RocksdbDatabase(shared_from_this(), info));
    }
}
