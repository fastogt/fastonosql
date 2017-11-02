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

#include "proxy/db/redis/server.h"

#include "core/db/redis/server_info.h"  // for ServerInfo, etc

#include "proxy/db/redis/database.h"  // for Database
#include "proxy/db/redis/driver.h"    // for Driver

#define MASTER_ROLE "master"
#define SLAVE_ROLE "slave"
#define STANDALONE_MODE "standalone"
#define SENTINEL_MODE "sentinel"
#define CLUSTER_MODE "cluster"

namespace fastonosql {
namespace proxy {
namespace redis {

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
  return IDatabaseSPtr(new Database(shared_from_this(), info));
}

void Server::HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  const events_info::DiscoveryInfoResponce v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    IServer::HandleDiscoveryInfoResponceEvent(ev);
    return;
  }

  core::IServerInfoSPtr serv_info = v.sinfo;
  core::redis::ServerInfo* rinf = static_cast<core::redis::ServerInfo*>(serv_info.get());
  if (rinf->replication_.role_ == MASTER_ROLE) {
    role_ = core::MASTER;
  } else if (rinf->replication_.role_ == SLAVE_ROLE) {
    role_ = core::SLAVE;
  }

  if (rinf->server_.redis_mode_ == STANDALONE_MODE) {
    mode_ = core::STANDALONE;
  } else if (rinf->server_.redis_mode_ == SENTINEL_MODE) {
    mode_ = core::SENTINEL;
  } else if (rinf->server_.redis_mode_ == CLUSTER_MODE) {
    mode_ = core::CLUSTER;
  }
  IServer::HandleDiscoveryInfoResponceEvent(ev);
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
