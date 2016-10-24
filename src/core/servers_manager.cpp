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

#include "core/servers_manager.h"

#include <stddef.h>  // for size_t

#include <algorithm>  // for remove
#include <memory>     // for __shared_ptr, shared_ptr, etc

#include <common/smart_ptr.h>  // for make_shared
#include <common/value.h>      // for ErrorValue, etc

#include "core/connection_types.h"  // for connectionTypes, etc
#include "core/cluster/icluster.h"
#include "core/sentinel/isentinel.h"  // for Sentinel

#ifdef BUILD_WITH_REDIS
#include "core/redis/cluster.h"              // for Cluster
#include "core/redis/sentinel.h"             // for Sentinel
#include "core/redis/connection_settings.h"  // for ConnectionSettings
#include "core/redis/db_connection.h"        // for discoveryClusterConnection, etc
#include "core/redis/server.h"               // for Server
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/connection_settings.h"  // for ConnectionSettings
#include "core/memcached/db_connection.h"        // for testConnection
#include "core/memcached/server.h"               // for Server
#endif

#ifdef BUILD_WITH_SSDB
#include "core/ssdb/connection_settings.h"  // for ConnectionSettings
#include "core/ssdb/db_connection.h"        // for testConnection
#include "core/ssdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/connection_settings.h"  // for ConnectionSettings
#include "core/leveldb/db_connection.h"        // for testConnection
#include "core/leveldb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/connection_settings.h"  // for ConnectionSettings
#include "core/rocksdb/db_connection.h"        // for testConnection
#include "core/rocksdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_LMDB
#include "core/lmdb/connection_settings.h"  // for ConnectionSettings
#include "core/lmdb/db_connection.h"        // for testConnection
#include "core/lmdb/server.h"               // for Server
#endif

#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/connection_settings.h"  // for ConnectionSettings
#include "core/unqlite/db_connection.h"        // for testConnection
#include "core/unqlite/server.h"               // for Server
#endif

namespace fastonosql {
namespace core {

ServersManager::ServersManager() {}

ServersManager::server_t ServersManager::createServer(IConnectionSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return server_t();
  }

  connectionTypes conT = settings->Type();
  server_t server;
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    server = common::make_shared<redis::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (conT == MEMCACHED) {
    server = common::make_shared<memcached::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (conT == SSDB) {
    server = common::make_shared<ssdb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (conT == LEVELDB) {
    server = common::make_shared<leveldb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (conT == ROCKSDB) {
    server = common::make_shared<rocksdb::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (conT == UNQLITE) {
    server = common::make_shared<unqlite::Server>(settings);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (conT == LMDB) {
    server = common::make_shared<lmdb::Server>(settings);
  }
#endif

  CHECK(server);
  servers_.push_back(server);
  return server;
}

ServersManager::sentinel_t ServersManager::createSentinel(ISentinelSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return sentinel_t();
  }

  connectionTypes conT = settings->Type();
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    sentinel_t sent = common::make_shared<redis::Sentinel>(settings->Path().ToString());
    auto nodes = settings->Sentinels();
    for (size_t i = 0; i < nodes.size(); ++i) {
      SentinelSettings nd = nodes[i];
      Sentinel sentt;
      IServerSPtr sent_serv = createServer(nd.sentinel);
      sentt.sentinel = sent_serv;
      for (size_t j = 0; j < nd.sentinel_nodes.size(); ++j) {
        IServerSPtr serv = createServer(nd.sentinel_nodes[j]);
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

ServersManager::cluster_t ServersManager::createCluster(IClusterSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return cluster_t();
  }

  connectionTypes conT = settings->Type();
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    cluster_t cl = common::make_shared<redis::Cluster>(settings->Path().ToString());
    auto nodes = settings->Nodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
      IConnectionSettingsBaseSPtr nd = nodes[i];
      IServerSPtr serv = createServer(nd);
      cl->AddServer(serv);
    }
    return cl;
  }
#endif

  NOTREACHED();
  return cluster_t();
}

common::Error ServersManager::testConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->Type();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return fastonosql::core::redis::testConnection(
        static_cast<redis::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return fastonosql::core::memcached::testConnection(
        static_cast<memcached::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return fastonosql::core::ssdb::testConnection(
        static_cast<ssdb::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return fastonosql::core::leveldb::testConnection(
        static_cast<leveldb::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return fastonosql::core::rocksdb::testConnection(
        static_cast<rocksdb::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return fastonosql::core::unqlite::testConnection(
        static_cast<unqlite::ConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return fastonosql::core::lmdb::testConnection(
        static_cast<lmdb::ConnectionSettings*>(connection.get()));
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

common::Error ServersManager::discoveryClusterConnection(
    IConnectionSettingsBaseSPtr connection,
    std::vector<ServerDiscoveryClusterInfoSPtr>* inf) {
  if (!connection || !inf) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->Type();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return redis::discoveryClusterConnection(
        static_cast<redis::ConnectionSettings*>(connection.get()), inf);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

common::Error ServersManager::discoverySentinelConnection(
    IConnectionSettingsBaseSPtr connection,
    std::vector<ServerDiscoverySentinelInfoSPtr>* inf) {
  if (!connection || !inf) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->Type();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return redis::discoverySentinelConnection(
        static_cast<redis::ConnectionSettings*>(connection.get()), inf);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return common::make_error_value("Not supported setting type", common::ErrorValue::E_ERROR);
  }
#endif

  NOTREACHED();
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

void ServersManager::clear() {
  servers_.clear();
}

void ServersManager::closeServer(server_t server) {
  servers_.erase(std::remove(servers_.begin(), servers_.end(), server));
}

void ServersManager::closeCluster(cluster_t cluster) {
  auto nodes = cluster->Nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    closeServer(nodes[i]);
  }
}

void ServersManager::closeSentinel(sentinel_t sentinel) {
  auto nodes = sentinel->Sentinels();
  for (auto node : nodes) {
    auto sent_nodes = node.sentinels_nodes;
    for (auto sent : sent_nodes) {
      closeServer(sent);
    }
  }
}

}  // namespace core
}  // namespace fastonosql
