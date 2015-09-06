#include "core/unqlite/unqlite_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    UnqliteDatabase::UnqliteDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == UNQLITE);
        DCHECK(info->type() == UNQLITE);
    }
}
