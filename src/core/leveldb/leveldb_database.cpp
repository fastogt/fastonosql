#include "core/leveldb/leveldb_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    LeveldbDatabase::LeveldbDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == LEVELDB);
        DCHECK(info->type() == LEVELDB);
    }
}
