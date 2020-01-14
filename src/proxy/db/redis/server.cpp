/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "proxy/db/redis/server.h"

#include <fastonosql/core/db/redis/server_info.h>

#include "proxy/db/redis/driver.h"
#include "proxy/db/redis_compatible/database.h"

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
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  Driver* drv = static_cast<Driver*>(drv_);
  VERIFY(QObject::connect(drv, &Driver::ModuleLoaded, this, &Server::LoadModule));
  VERIFY(QObject::connect(drv, &Driver::ModuleUnLoaded, this, &Server::UnLoadModule));
#endif

  StartCheckKeyExistTimer();
}

Server::~Server() {
  StopCheckKeyExistTimer();
}

core::ServerType Server::GetRole() const {
  return role_;
}

core::ServerMode Server::GetMode() const {
  return mode_;
}

core::ServerState Server::GetState() const {
  return core::SUP;
}

common::net::HostAndPort Server::GetHost() const {
  Driver* rdrv = static_cast<Driver*>(drv_);
  return rdrv->GetHost();
}

IDatabaseSPtr Server::CreateDatabase(core::IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new redis_compatible::Database(shared_from_this(), info));
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
void Server::LoadModule(core::ModuleInfo module) {
  emit ModuleLoaded(module);
}

void Server::UnLoadModule(core::ModuleInfo module) {
  emit ModuleUnLoaded(module);
}
#endif

void Server::HandleLoadServerInfoEvent(events::ServerInfoResponseEvent* ev) {
  const events_info::ServerInfoResponse v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    IServer::HandleLoadServerInfoEvent(ev);
    return;
  }

  core::IServerInfoSPtr serv_info = v.GetInfo();
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
  IServer::HandleLoadServerInfoEvent(ev);
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
