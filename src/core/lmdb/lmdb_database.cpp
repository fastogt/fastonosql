#include "core/lmdb/lmdb_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    LmdbDatabase::LmdbDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == LMDB);
        DCHECK(info->type() == LMDB);
    }
}
