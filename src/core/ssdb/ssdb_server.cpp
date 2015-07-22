#include "core/ssdb/ssdb_server.h"

#include "core/ssdb/ssdb_database.h"

namespace fastonosql
{
    SsdbServer::SsdbServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv,isSuperServer)
    {

    }

    IDatabaseSPtr SsdbServer::createDatabaseImpl(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new SsdbDatabase(shared_from_this(), info));
    }
}
