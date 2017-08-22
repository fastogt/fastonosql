/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/ssdb/server.h"

#include "proxy/db/ssdb/database.h"
#include "proxy/db/ssdb/driver.h"

namespace fastonosql {
namespace proxy {
namespace ssdb {

Server::Server(IConnectionSettingsBaseSPtr settings) : IServerRemote(new Driver(settings)) {
  StartCheckKeyExistTimer();
}

Server::~Server() {
  StopCheckKeyExistTimer();
}

core::serverMode Server::Mode() const {
  return core::STANDALONE;
}

core::serverTypes Server::Role() const {
  return core::MASTER;
}

core::serverState Server::State() const {
  return core::SUP;
}

common::net::HostAndPort Server::GetHost() const {
  Driver* const rdrv = static_cast<Driver* const>(drv_);
  return rdrv->GetHost();
}

IDatabaseSPtr Server::CreateDatabase(core::IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new Database(shared_from_this(), info));
}

}  // namespace ssdb
}  // namespace proxy
}  // namespace fastonosql
