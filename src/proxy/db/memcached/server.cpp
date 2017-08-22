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

#include "proxy/db/memcached/server.h"

#include "proxy/db/memcached/database.h"
#include "proxy/db/memcached/driver.h"

namespace fastonosql {
namespace proxy {
namespace memcached {

Server::Server(IConnectionSettingsBaseSPtr settings) : IServerRemote(new Driver(settings)) {
  StartCheckKeyExistTimer();
}

Server::~Server() {
  StopCheckKeyExistTimer();
}

core::serverTypes Server::Role() const {
  return core::MASTER;
}

core::serverMode Server::Mode() const {
  return core::STANDALONE;
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

}  // namespace memcached
}  // namespace proxy
}  // namespace fastonosql
