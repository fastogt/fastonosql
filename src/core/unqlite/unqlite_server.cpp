#include "core/unqlite/unqlite_server.h"

#include "core/unqlite/unqlite_database.h"

namespace fastonosql
{
    UnqliteServer::UnqliteServer(const IDriverSPtr& drv, bool isSuperServer)
        : IServer(drv, isSuperServer)
    {

    }

    IDatabaseSPtr UnqliteServer::createDatabase(DataBaseInfoSPtr info)
    {
        return IDatabaseSPtr(new UnqliteDatabase(shared_from_this(), info));
    }
}
