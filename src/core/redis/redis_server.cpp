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

#include "core/redis/redis_server.h"

#include "core/redis/redis_driver.h"
#include "core/redis/database.h"

namespace fastonosql {
namespace core {
namespace redis {

RedisServer::RedisServer(IConnectionSettingsBaseSPtr settings)
  : IServerRemote(new RedisDriver(settings)), role_(MASTER), mode_(STANDALONE) {
}

serverTypes RedisServer::role() const {
  return role_;
}

serverMode RedisServer::mode() const {
  return mode_;
}

common::net::hostAndPort RedisServer::host() const {
  RedisDriver* const rdrv = static_cast<RedisDriver* const>(drv_);
  return rdrv->host();
}

IDatabaseSPtr RedisServer::createDatabase(IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new Database(shared_from_this(), info));
}

void RedisServer::handleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (!er) {
    ServerInfo* rinf = dynamic_cast<ServerInfo*>(v.sinfo.get());  // +
    CHECK(rinf);
    if (rinf->replication_.role_ == "master") {
      role_ = MASTER;
    } else if(rinf->replication_.role_ == "slave") {
      role_ = SLAVE;
    }

    if (rinf->server_.redis_mode_ == "standalone") {
      mode_ = STANDALONE;
    } else if (rinf->server_.redis_mode_ == "sentinel") {
      mode_ = SENTINEL;
    } else if (rinf->server_.redis_mode_ == "cluster") {
      mode_ = CLUSTER;
    }
  }
  IServer::handleDiscoveryInfoResponceEvent(ev);
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
