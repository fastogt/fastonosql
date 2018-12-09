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

#include "proxy/servers_manager.h"

#include <vector>

#if defined(BUILD_WITH_REDIS)
#include <fastonosql/core/db/redis/db_connection.h>  // for DiscoveryClusterConnection, etc
#include "proxy/db/redis/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/redis/server.h"                   // for Server

#if defined(PRO_VERSION)
#include "proxy/db/redis/cluster.h"   // for Cluster
#include "proxy/db/redis/sentinel.h"  // for Sentinel
#endif
#endif

#if defined(BUILD_WITH_MEMCACHED)
#include <fastonosql/core/db/memcached/db_connection.h>  // for TestConnection
#include "proxy/db/memcached/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/memcached/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_SSDB)
#include <fastonosql/core/db/ssdb/db_connection.h>  // for TestConnection
#include "proxy/db/ssdb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/ssdb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_LEVELDB)
#include <fastonosql/core/db/leveldb/db_connection.h>  // for TestConnection
#include "proxy/db/leveldb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/leveldb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_ROCKSDB)
#include <fastonosql/core/db/rocksdb/db_connection.h>  // for TestConnection
#include "proxy/db/rocksdb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/rocksdb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_UNQLITE)
#include <fastonosql/core/db/unqlite/db_connection.h>  // for TestConnection
#include "proxy/db/unqlite/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/unqlite/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_LMDB)
#include <fastonosql/core/db/lmdb/db_connection.h>  // for TestConnection
#include "proxy/db/lmdb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/lmdb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_UPSCALEDB)
#include <fastonosql/core/db/upscaledb/db_connection.h>  // for TestConnection
#include "proxy/db/upscaledb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/upscaledb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_FORESTDB)
#include <fastonosql/core/db/forestdb/db_connection.h>  // for TestConnection
#include "proxy/db/forestdb/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/forestdb/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_PIKA)
#include <fastonosql/core/db/pika/db_connection.h>  // for DiscoveryClusterConnection, etc
#include "proxy/db/pika/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/pika/server.h"                   // for Server
#endif

#if defined(BUILD_WITH_DYNOMITE)
#include <fastonosql/core/db/dynomite/db_connection.h>  // for DiscoveryClusterConnection, etc
#include "proxy/db/dynomite/connection_settings.h"      // for ConnectionSettings
#include "proxy/db/dynomite/server.h"                   // for Server
#endif

#if defined(PRO_VERSION)
#include "proxy/cluster/icluster.h"
#include "proxy/sentinel/isentinel.h"  // for Sentinel
#endif

namespace fastonosql {
namespace proxy {

namespace {
IServerSPtr CreateServerImpl(IConnectionSettingsBaseSPtr settings) {
  const core::ConnectionType connection_type = settings->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    return std::make_shared<redis::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (connection_type == core::MEMCACHED) {
    return std::make_shared<memcached::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (connection_type == core::SSDB) {
    return std::make_shared<ssdb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (connection_type == core::LEVELDB) {
    return std::make_shared<leveldb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (connection_type == core::ROCKSDB) {
    return std::make_shared<rocksdb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (connection_type == core::UNQLITE) {
    return std::make_shared<unqlite::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (connection_type == core::LMDB) {
    return std::make_shared<lmdb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (connection_type == core::UPSCALEDB) {
    return std::make_shared<upscaledb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (connection_type == core::FORESTDB) {
    return std::make_shared<forestdb::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (connection_type == core::PIKA) {
    return std::make_shared<pika::Server>(settings);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (connection_type == core::DYNOMITE) {
    return std::make_shared<dynomite::Server>(settings);
  }
#endif

  NOTREACHED() << "Server should be allocated, type: " << connection_type;
  return IServerSPtr();
}
}  // namespace

ServersManager::ServersManager() : servers_() {}

ServersManager::server_t ServersManager::CreateServer(IConnectionSettingsBaseSPtr settings) {
  CHECK(settings);

  const server_t server = CreateServerImpl(settings);
  servers_.push_back(server);
  return server;
}

common::Error ServersManager::TestConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  const core::ConnectionType connection_type = connection->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::TestConnection(rconfig);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (connection_type == core::MEMCACHED) {
    memcached::ConnectionSettings* settings = static_cast<memcached::ConnectionSettings*>(connection.get());
    return core::memcached::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (connection_type == core::SSDB) {
    ssdb::ConnectionSettings* settings = static_cast<ssdb::ConnectionSettings*>(connection.get());
    return core::ssdb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (connection_type == core::LEVELDB) {
    leveldb::ConnectionSettings* settings = static_cast<leveldb::ConnectionSettings*>(connection.get());
    return core::leveldb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (connection_type == core::ROCKSDB) {
    rocksdb::ConnectionSettings* settings = static_cast<rocksdb::ConnectionSettings*>(connection.get());
    return core::rocksdb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (connection_type == core::UNQLITE) {
    unqlite::ConnectionSettings* settings = static_cast<unqlite::ConnectionSettings*>(connection.get());
    return core::unqlite::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (connection_type == core::LMDB) {
    lmdb::ConnectionSettings* settings = static_cast<lmdb::ConnectionSettings*>(connection.get());
    return core::lmdb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (connection_type == core::UPSCALEDB) {
    upscaledb::ConnectionSettings* settings = static_cast<upscaledb::ConnectionSettings*>(connection.get());
    return core::upscaledb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (connection_type == core::FORESTDB) {
    forestdb::ConnectionSettings* settings = static_cast<forestdb::ConnectionSettings*>(connection.get());
    return core::forestdb::TestConnection(settings->GetInfo());
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (connection_type == core::PIKA) {
    pika::ConnectionSettings* settings = static_cast<pika::ConnectionSettings*>(connection.get());
    core::pika::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::pika::TestConnection(rconfig);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (connection_type == core::DYNOMITE) {
    dynomite::ConnectionSettings* settings = static_cast<dynomite::ConnectionSettings*>(connection.get());
    core::dynomite::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::dynomite::TestConnection(rconfig);
  }
#endif

  NOTREACHED() << "Can't find test connection implementation for: " << connection_type;
  return common::make_error("Invalid setting type");
}

void ServersManager::Clear() {
  servers_.clear();
}

void ServersManager::CloseServer(server_t server) {
  CHECK(server);

  servers_.erase(std::remove(servers_.begin(), servers_.end(), server));
}

#if defined(PRO_VERSION)
common::Error ServersManager::DiscoveryClusterConnection(IConnectionSettingsBaseSPtr connection,
                                                         std::vector<core::ServerDiscoveryClusterInfoSPtr>* out) {
  if (!connection || !out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  const core::ConnectionType connection_type = connection->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::DiscoveryClusterConnection(rconfig, out);
  }
#endif

  NOTREACHED() << "Can't find discovery cluster implementation for: " << connection_type;
  return common::make_error("Invalid setting type");
}

common::Error ServersManager::DiscoverySentinelConnection(IConnectionSettingsBaseSPtr connection,
                                                          std::vector<core::ServerDiscoverySentinelInfoSPtr>* out) {
  if (!connection || !out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  const core::ConnectionType connection_type = connection->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    redis::ConnectionSettings* settings = static_cast<redis::ConnectionSettings*>(connection.get());
    core::redis::RConfig rconfig(settings->GetInfo(), settings->GetSSHInfo());
    return core::redis::DiscoverySentinelConnection(rconfig, out);
  }
#endif

  NOTREACHED() << "Can't find discovery cluster implementation for: " << connection_type;
  return common::make_error("Invalid setting type");
}

ServersManager::sentinel_t ServersManager::CreateSentinel(ISentinelSettingsBaseSPtr settings) {
  CHECK(settings);

  const core::ConnectionType connection_type = settings->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    sentinel_t sent = std::make_shared<redis::Sentinel>(settings->GetPath().ToString());
    auto nodes = settings->GetSentinels();
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

  NOTREACHED() << "Sentinel should be allocated, type: " << connection_type;
  return sentinel_t();
}

ServersManager::cluster_t ServersManager::CreateCluster(IClusterSettingsBaseSPtr settings) {
  CHECK(settings);

  const core::ConnectionType connection_type = settings->GetType();
#if defined(BUILD_WITH_REDIS)
  if (connection_type == core::REDIS) {
    cluster_t cl = std::make_shared<redis::Cluster>(settings->GetPath().ToString());
    auto nodes = settings->GetNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
      IConnectionSettingsBaseSPtr nd = nodes[i];
      IServerSPtr serv = CreateServer(nd);
      cl->AddServer(serv);
    }
    return cl;
  }
#endif

  NOTREACHED() << "Cluster should be allocated, type: " << connection_type;
  return cluster_t();
}

void ServersManager::CloseCluster(cluster_t cluster) {
  CHECK(cluster);

  auto nodes = cluster->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    CloseServer(nodes[i]);
  }
}

void ServersManager::CloseSentinel(sentinel_t sentinel) {
  CHECK(sentinel);

  auto nodes = sentinel->GetSentinels();
  for (auto node : nodes) {
    auto sent_nodes = node.sentinels_nodes;
    for (auto sent : sent_nodes) {
      CloseServer(sent);
    }
  }
}
#endif

}  // namespace proxy
}  // namespace fastonosql
