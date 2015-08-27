#include "core/rocksdb/rocksdb_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    RocksdbDatabase::RocksdbDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == ROCKSDB);
        DCHECK(info->type() == ROCKSDB);
    }
}
