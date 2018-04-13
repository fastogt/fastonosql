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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/pika/server.h"

#include "core/db/pika/server_info.h"  // for ServerInfo, etc

#include "proxy/db/pika/driver.h"                // for Driver
#include "proxy/db/redis_compatible/database.h"  // for Database

#define MASTER_ROLE "master"
#define SLAVE_ROLE "slave"
#define STANDALONE_MODE "standalone"
#define SENTINEL_MODE "sentinel"
#define CLUSTER_MODE "cluster"

namespace fastonosql {
namespace proxy {
namespace pika {

Server::Server(IConnectionSettingsBaseSPtr settings)
    : IServerRemote(new Driver(settings)), role_(core::MASTER), mode_(core::STANDALONE) {
  StartCheckKeyExistTimer();
}

Server::~Server() {
  StopCheckKeyExistTimer();
}

core::serverTypes Server::GetRole() const {
  return role_;
}

core::serverMode Server::GetMode() const {
  return mode_;
}

core::serverState Server::GetState() const {
  return core::SUP;
}

common::net::HostAndPort Server::GetHost() const {
  Driver* const rdrv = static_cast<Driver* const>(drv_);
  return rdrv->GetHost();
}

IDatabaseSPtr Server::CreateDatabase(core::IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new redis_compatible::Database(shared_from_this(), info));
}

void Server::HandleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev) {
  const events_info::ServerInfoResponce v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    IServer::HandleLoadServerInfoEvent(ev);
    return;
  }

  core::IServerInfoSPtr serv_info = v.info();
  core::pika::ServerInfo* rinf = static_cast<core::pika::ServerInfo*>(serv_info.get());
  if (rinf->replication_.role_ == MASTER_ROLE) {
    role_ = core::MASTER;
  } else if (rinf->replication_.role_ == SLAVE_ROLE) {
    role_ = core::SLAVE;
  }

  IServer::HandleLoadServerInfoEvent(ev);
}

}  // namespace pika
}  // namespace proxy
}  // namespace fastonosql
