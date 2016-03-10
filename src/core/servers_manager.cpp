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

#include <vector>

#include "core/icluster.h"

#ifdef BUILD_WITH_REDIS
#include "core/redis/redis_cluster.h"
#include "core/redis/redis_server.h"
#include "core/redis/redis_driver.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/memcached_server.h"
#include "core/memcached/memcached_raw.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "core/ssdb/ssdb_server.h"
#include "core/ssdb/ssdb_raw.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/leveldb_server.h"
#include "core/leveldb/leveldb_raw.h"
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/rocksdb_server.h"
#include "core/rocksdb/rocksdb_raw.h"
#endif

#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/unqlite_server.h"
#include "core/unqlite/unqlite_raw.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "core/lmdb/lmdb_server.h"
#include "core/lmdb/lmdb_raw.h"
#endif

namespace fastonosql {

ServersManager::ServersManager() {
  qRegisterMetaType<ServerInfoSnapShoot>("ServerInfoSnapShoot");
}

ServersManager::~ServersManager() {
}

ServersManager::server_t ServersManager::createServer(IConnectionSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return IServerSPtr();
  }

  connectionTypes conT = settings->type();
  IServer* server = nullptr;
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    server = new redis::RedisServer(settings);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (conT == MEMCACHED) {
    server = new memcached::MemcachedServer(settings);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (conT == SSDB) {
    server = new ssdb::SsdbServer(settings);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (conT == LEVELDB) {
    server = new leveldb::LeveldbServer(settings);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (conT == ROCKSDB) {
    server = new rocksdb::RocksdbServer(settings);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (conT == UNQLITE) {
    server = new unqlite::UnqliteServer(settings);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (conT == LMDB) {
    server = new lmdb::LmdbServer(settings);
  }
#endif

  if (!server) {
    NOTREACHED();
    return IServerSPtr();
  }

  IServerSPtr sh(server);
  servers_.push_back(sh);
  return sh;
}

ServersManager::cluster_t ServersManager::createCluster(IClusterSettingsBaseSPtr settings) {
  if (!settings) {
    NOTREACHED();
    return IClusterSPtr();
  }

  connectionTypes conT = settings->type();
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    IClusterSPtr cl(new redis::RedisCluster(settings->name()));
    IClusterSettingsBase::cluster_connection_type nodes = settings->nodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
      IConnectionSettingsBaseSPtr nd = nodes[i];
      IServerSPtr serv = createServer(nd);
      cl->addServer(serv);
    }
    return cl;
  }
#endif

  NOTREACHED();
  return IClusterSPtr();
}

common::Error ServersManager::testConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    NOTREACHED();
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->type();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return fastonosql::redis::testConnection(dynamic_cast<redis::RedisConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return fastonosql::memcached::testConnection(dynamic_cast<memcached::MemcachedConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return fastonosql::ssdb::testConnection(dynamic_cast<ssdb::SsdbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return fastonosql::leveldb::testConnection(dynamic_cast<leveldb::LeveldbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return fastonosql::rocksdb::testConnection(dynamic_cast<rocksdb::RocksdbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return fastonosql::unqlite::testConnection(dynamic_cast<unqlite::UnqliteConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return fastonosql::lmdb::testConnection(dynamic_cast<lmdb::LmdbConnectionSettings*>(connection.get()));
  }
#endif
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

common::Error ServersManager::discoveryConnection(IConnectionSettingsBaseSPtr connection,
                                                  std::vector<ServerDiscoveryInfoSPtr>* inf) {
  if (!connection || !inf) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->type();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return fastonosql::redis::discoveryConnection(dynamic_cast<redis::RedisConnectionSettings*>(connection.get()), inf);
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
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

void ServersManager::clear() {
  servers_.clear();
}

void ServersManager::closeServer(server_t server) {
  servers_.erase(std::remove(servers_.begin(), servers_.end(), server));
}

void ServersManager::closeCluster(cluster_t cluster) {
  ICluster::nodes_type nodes = cluster->nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    closeServer(nodes[i]);
  }
}

}  // namespace fastonosql
