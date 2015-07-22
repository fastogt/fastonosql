#include "core/ssdb/ssdb_database.h"

#include "core/iserver.h"

namespace fastonosql
{
    SsdbDatabase::SsdbDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : IDatabase(server, info)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == SSDB);
        DCHECK(info->type() == SSDB);
    }
}
