/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/ssdb/ssdb_server.h"

#include "core/ssdb/ssdb_database.h"

namespace fastonosql {

SsdbServer::SsdbServer(const IDriverSPtr& drv, bool isSuperServer)
  : IServer(drv,isSuperServer) {
}

IDatabaseSPtr SsdbServer::createDatabase(DataBaseInfoSPtr info) {
  return IDatabaseSPtr(new SsdbDatabase(shared_from_this(), info));
}

}
