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

#include "proxy/servers_manager.h"

#include "proxy/cluster/icluster.h"
#include "proxy/sentinel/isentinel.h"  // for Sentinel

#ifdef BUILD_WITH_REDIS
#include "core/db/redis/db_connection.h"         // for DiscoveryClusterConnection, etc
#include "proxy/db/redis/cluster.h"              // for Cluster
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/redis/sentinel.h"             // for Sentinel
#include "proxy/db/redis/server.h"               // for Server
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/db/memcached/db_connection.h"         // for TestConnection
#include "proxy/db/memcached/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/memcached/server.h"               // for Server
#endif

#ifdef BUILD_WITH_SSDB
#include "core/db/ssdb/db_connection.h"         // for TestConnection
#include "proxy/db/ssdb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/ssdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/db/leveldb/db_connection.h"         // for TestConnection
#include "proxy/db/leveldb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/leveldb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "core/db/rocksdb/db_connection.h"         // for TestConnection
#include "proxy/db/rocksdb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/rocksdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_UNQLITE
#include "core/db/unqlite/db_connection.h"         // for TestConnection
#include "proxy/db/unqlite/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/unqlite/server.h"               // for Server
#endif

#ifdef BUILD_WITH_LMDB
#include "core/db/lmdb/db_connection.h"         // for TestConnection
#include "proxy/db/lmdb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/lmdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_UPSCALEDB
#include "core/db/upscaledb/db_connection.h"         // for TestConnection
#include "proxy/db/upscaledb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/upscaledb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_FORESTDB
#include "core/db/forestdb/db_connection.h"         // for TestConnection
#include "proxy/db/forestdb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/forestdb/server.h"               // for Server
#endif

namespace fastonosql {
namespace proxy {

ServersManager::ServersManager() {}

ServersManager::server_t ServersManager::CreateServer(IConnectionSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return server_t();
  }

  core::connectionTypes conT = settings->GetType();
  server_t server;
#ifdef BUILD_WITH_REDIS
  if (conT == core::REDIS) {
    server = std::make_shared<redis::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (conT == core::MEMCACHED) {
    server = std::make_shared<memcached::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (conT == core::SSDB) {
    server = std::make_shared<ssdb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (conT == core::LEVELDB) {
    server = std::make_shared<leveldb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (conT == core::ROCKSDB) {
    server = std::make_shared<rocksdb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (conT == core::UNQLITE) {
    server = std::make_shared<unqlite::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (conT == core::LMDB) {
    server = std::make_shared<lmdb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (conT == core::UPSCALEDB) {
    server = std::make_shared<upscaledb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (conT == core::FORESTDB) {
    server = std::make_shared<forestdb::Server>(settings);
  }
#endif

  CHECK(server);
  servers_.push_back(server);
  return server;
}

ServersManager::sentinel_t ServersManager::CreateSentinel(ISentinelSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return sentinel_t();
  }

  core::connectionTypes conT = settings->GetType();
#ifdef BUILD_WITH_REDIS
  if (conT == core::REDIS) {
    sentinel_t sent = std::make_shared<redis::Sentinel>(settings->Path().ToString());
    auto nodes = settings->Sentinels();
    for (size_t i = 0; i < nodes.size(); ++i) {
      SentinelSettings nd = nodes[i];
      Sentinel sentt;
      IServerSPtr sent_serv = CreateServer(nd.sentinel);
      sentt.sentinel = sent_serv;
      for (size_t j = 0; j < nd.sentinel_nodes.size(); ++j) {
        IServerSPtr serv = CreateServer(nd.sentinel_nodes[j]);
        sentt.sentinels_nodes.push_back(serv);
      }

      sent->AddSentinel(sentt);
    }
    return sent;
  }
#endif

  NOTREACHED();
  return sentinel_t();
}

ServersManager::cluster_t ServersManager::CreateCluster(IClusterSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return cluster_t();
  }

  core::connectionTypes conT = settings->GetType();
#ifdef BUILD_WITH_REDIS
  if (conT == core::REDIS) {
    cluster_t cl = std::make_shared<redis::Cluster>(settings->Path().ToString());
    auto nodes = settings->Nodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
      IConnectionSettingsBaseSPtr nd = nodes[i];
      IServerSPtr serv = CreateServer(nd);
      cl->AddServer(serv);
    }
    return cl;
  }
#endif

  NOTREACHED();
  return cluster_t();
}

common::Error ServersManager::TestConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  core::connectionTypes type = connection->GetType();
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::TestConnection(rconfig);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    memcached::ConnectionSettings* settings = static_cast<memcached::ConnectionSettings*>(connection.get());
    return fastonosql::core::memcached::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    ssdb::ConnectionSettings* settings = static_cast<ssdb::ConnectionSettings*>(connection.get());
    return fastonosql::core::ssdb::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    leveldb::ConnectionSettings* settings = static_cast<leveldb::ConnectionSettings*>(connection.get());
    return fastonosql::core::leveldb::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    rocksdb::ConnectionSettings* settings = static_cast<rocksdb::ConnectionSettings*>(connection.get());
    return fastonosql::core::rocksdb::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    unqlite::ConnectionSettings* settings = static_cast<unqlite::ConnectionSettings*>(connection.get());
    return fastonosql::core::unqlite::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    lmdb::ConnectionSettings* settings = static_cast<lmdb::ConnectionSettings*>(connection.get());
    return fastonosql::core::lmdb::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    upscaledb::ConnectionSettings* settings = static_cast<upscaledb::ConnectionSettings*>(connection.get());
    return fastonosql::core::upscaledb::TestConnection(settings->Info());
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    forestdb::ConnectionSettings* settings = static_cast<forestdb::ConnectionSettings*>(connection.get());
    return fastonosql::core::forestdb::TestConnection(settings->Info());
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ERROR_TYPE);
}

common::Error ServersManager::DiscoveryClusterConnection(IConnectionSettingsBaseSPtr connection,
                                                         std::vector<core::ServerDiscoveryClusterInfoSPtr>* inf) {
  if (!connection || !inf) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  core::connectionTypes type = connection->GetType();
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::DiscoveryClusterConnection(rconfig, inf);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ERROR_TYPE);
}

common::Error ServersManager::DiscoverySentinelConnection(IConnectionSettingsBaseSPtr connection,
                                                          std::vector<core::ServerDiscoverySentinelInfoSPtr>* inf) {
  if (!connection || !inf) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  core::connectionTypes type = connection->GetType();
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::DiscoverySentinelConnection(rconfig, inf);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    return common::make_error_value("Not supported setting type", common::ERROR_TYPE);
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ERROR_TYPE);
}

void ServersManager::Clear() {
  servers_.clear();
}

void ServersManager::CloseServer(server_t server) {
  servers_.erase(std::remove(servers_.begin(), servers_.end(), server));
}

void ServersManager::CloseCluster(cluster_t cluster) {
  auto nodes = cluster->Nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    CloseServer(nodes[i]);
  }
}

void ServersManager::CloseSentinel(sentinel_t sentinel) {
  auto nodes = sentinel->Sentinels();
  for (auto node : nodes) {
    auto sent_nodes = node.sentinels_nodes;
    for (auto sent : sent_nodes) {
      CloseServer(sent);
    }
  }
}

}  // namespace proxy
}  // namespace fastonosql
