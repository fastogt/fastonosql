/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <string>  // for operator==, basic_string
#include <memory>  // for __shared_ptr

#include <common/error.h>   // for Error
#include <common/macros.h>  // for CHECK
#include <common/value.h>   // for ErrorValue

#include "core/server/iserver_info.h"
#include "core/db/redis/server_info.h"  // for ServerInfo, etc

#include "proxy/server/iserver.h"      // for IServer
#include "proxy/events/events_info.h"  // for DiscoveryInfoResponce
#include "proxy/db/redis/database.h"   // for Database
#include "proxy/db/redis/driver.h"     // for Driver

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

core::serverTypes Server::Role() const {
  return role_;
}

core::serverMode Server::Mode() const {
  return mode_;
}

core::serverState Server::State() const {
  return core::SUP;
}

common::net::HostAndPort Server::Host() const {
  Driver* const rdrv = static_cast<Driver* const>(drv_);
  return rdrv->Host();
}

IDatabaseSPtr Server::CreateDatabase(core::IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new Database(shared_from_this(), info));
}

void Server::HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  const events_info::DiscoveryInfoResponce v = ev->value();
  common::Error err = v.errorInfo();
  if (err && err->IsError()) {
    IServer::HandleDiscoveryInfoResponceEvent(ev);
    return;
  }

  core::IServerInfoSPtr serv_info = v.sinfo;
  core::redis::ServerInfo* rinf = static_cast<core::redis::ServerInfo*>(serv_info.get());
  if (rinf->replication_.role_ == "master") {
    role_ = core::MASTER;
  } else if (rinf->replication_.role_ == "slave") {
    role_ = core::SLAVE;
  }

  if (rinf->server_.redis_mode_ == "standalone") {
    mode_ = core::STANDALONE;
  } else if (rinf->server_.redis_mode_ == "sentinel") {
    mode_ = core::SENTINEL;
  } else if (rinf->server_.redis_mode_ == "cluster") {
    mode_ = core::CLUSTER;
  }
  IServer::HandleDiscoveryInfoResponceEvent(ev);
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
