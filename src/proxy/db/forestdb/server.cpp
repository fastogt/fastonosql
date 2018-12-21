/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/forestdb/server.h"

#include "proxy/db/forestdb/database.h"
#include "proxy/db/forestdb/driver.h"

namespace fastonosql {
namespace proxy {
namespace forestdb {

Server::Server(IConnectionSettingsBaseSPtr settings) : IServerLocal(new Driver(settings)) {}

std::string Server::GetPath() const {
  Driver* ldrv = static_cast<Driver*>(drv_);
  return ldrv->GetPath();
}

IDatabaseSPtr Server::CreateDatabase(core::IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new Database(shared_from_this(), info));
}

}  // namespace forestdb
}  // namespace proxy
}  // namespace fastonosql
