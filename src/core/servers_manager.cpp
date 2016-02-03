/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/servers_manager.h"

#include <vector>

#include "core/settings_manager.h"

#include "core/icluster.h"

#ifdef BUILD_WITH_REDIS
#include "core/redis/redis_cluster.h"
#include "core/redis/redis_server.h"
#include "core/redis/redis_driver.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/memcached_server.h"
#include "core/memcached/memcached_driver.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "core/ssdb/ssdb_server.h"
#include "core/ssdb/ssdb_driver.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/leveldb_server.h"
#include "core/leveldb/leveldb_driver.h"
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/rocksdb_server.h"
#include "core/rocksdb/rocksdb_driver.h"
#endif

#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/unqlite_server.h"
#include "core/unqlite/unqlite_driver.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "core/lmdb/lmdb_server.h"
#include "core/lmdb/lmdb_driver.h"
#endif

namespace fastonosql {

ServersManager::ServersManager()
  : sync_servers_(SettingsManager::instance().syncTabs()) {
  qRegisterMetaType<ServerInfoSnapShoot>("ServerInfoSnapShoot");
}

ServersManager::~ServersManager() {
}

template<class Server, class Driver>
IServer* ServersManager::make_server(IServerSPtr pser, IConnectionSettingsBaseSPtr settings) {
  if (!pser) {
    IDriverSPtr dr(new Driver(settings));
    dr->start();
    return new Server(dr, true);
  }

  return new Server(pser->driver(), false);
}

IServerSPtr ServersManager::createServer(IConnectionSettingsBaseSPtr settings) {
  DCHECK(settings);

  IServerSPtr result;
  connectionTypes conT = settings->connectionType();
  IServerSPtr ser = findServerBySetting(settings);
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    result.reset(make_server<RedisServer, RedisDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (conT == MEMCACHED) {
    result.reset(make_server<MemcachedServer, MemcachedDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (conT == SSDB) {
    result.reset(make_server<SsdbServer, SsdbDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (conT == LEVELDB) {
    result.reset(make_server<LeveldbServer, LeveldbDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (conT == ROCKSDB) {
    result.reset(make_server<RocksdbServer, RocksdbDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (conT == UNQLITE) {
    result.reset(make_server<UnqliteServer, UnqliteDriver>(ser, settings));
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (conT == LMDB) {
    result.reset(make_server<LmdbServer, LmdbDriver>(ser, settings));
  }
#endif
  DCHECK(result);
  if (result) {
    servers_.push_back(result);
    if (ser && sync_servers_) {
      result->syncWithServer(ser.get());
    }
  }

  return result;
}

IClusterSPtr ServersManager::createCluster(IClusterSettingsBaseSPtr settings) {
  DCHECK(settings);

  IClusterSPtr cl;
  connectionTypes conT = settings->connectionType();
#ifdef BUILD_WITH_REDIS
  if (conT == REDIS) {
    IConnectionSettingsBaseSPtr root = settings->root();
    if (!root) {
      return IClusterSPtr();
    }

    cl.reset(new RedisCluster(settings->connectionName()));
    IClusterSettingsBase::cluster_connection_type nodes = settings->nodes();
    for (int i = 0; i < nodes.size(); ++i) {
      IConnectionSettingsBaseSPtr nd = nodes[i];
      if (nd) {
        IServerSPtr serv = createServer(nd);
        cl->addServer(serv);
      }
    }
    IDriverSPtr drv = cl->root()->driver();
    DCHECK(drv->settings() == root);
  }
#endif

  return cl;
}

common::Error ServersManager::testConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->connectionType();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return fastonosql::testConnection(dynamic_cast<RedisConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return fastonosql::testConnection(dynamic_cast<MemcachedConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return fastonosql::testConnection(dynamic_cast<SsdbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return fastonosql::testConnection(dynamic_cast<LeveldbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return fastonosql::testConnection(dynamic_cast<RocksdbConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return fastonosql::testConnection(dynamic_cast<UnqliteConnectionSettings*>(connection.get()));
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return fastonosql::testConnection(dynamic_cast<UnqliteConnectionSettings*>(connection.get()));
  }
#endif
  return common::make_error_value("Invalid setting type", common::ErrorValue::E_ERROR);
}

common::Error ServersManager::discoveryConnection(IConnectionSettingsBaseSPtr connection,
                                                  std::vector<ServerDiscoveryInfoSPtr>& inf) {
  if (!connection) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  connectionTypes type = connection->connectionType();
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return fastonosql::discoveryConnection(dynamic_cast<RedisConnectionSettings*>(connection.get()), inf);
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

void ServersManager::setSyncServers(bool isSync) {
  sync_servers_ = isSync;
  refreshSyncServers();
}

void ServersManager::clear() {
  for (size_t i = 0; i < servers_.size(); ++i) {
    IServerSPtr ser = servers_[i];
    ser->driver()->stop();
  }
  servers_.clear();
}

void ServersManager::closeServer(IServerSPtr server) {
  for (size_t i = 0; i < servers_.size(); ++i) {
    IServerSPtr ser = servers_[i];
    if (ser == server) {
      if (ser->isSuperServer()) {
        IDriverSPtr drv = ser->driver();
        for (size_t j = 0; j < servers_.size(); ++j) {
          IServerSPtr servj = servers_[j];
          if (servj->driver() == drv) {
            servj->is_super_server_ = true;
            break;
          }
        }
      }

      servers_.erase(servers_.begin()+i);
      refreshSyncServers();
      break;
    }
  }
}

void ServersManager::closeCluster(IClusterSPtr cluster) {
  ICluster::nodes_type nodes = cluster->nodes();
  for (int i = 0; i < nodes.size(); ++i) {
    closeServer(nodes[i]);
  }
}

void ServersManager::refreshSyncServers() {
  for (size_t i = 0; i < servers_.size(); ++i) {
    IServerSPtr servi = servers_[i];
    if (servi->isSuperServer()) {
      for (size_t j = 0; j < servers_.size(); ++j) {
        IServerSPtr servj = servers_[j];
        if (servj != servi && servj->driver() == servi->driver()) {
          if (sync_servers_) {
            servj->syncWithServer(servi.get());
          } else {
            servj->unSyncFromServer(servi.get());
          }
        }
      }
    }
  }
}

IServerSPtr ServersManager::findServerBySetting(const IConnectionSettingsBaseSPtr &settings) const {
  for (size_t i = 0; i < servers_.size(); ++i) {
    IServerSPtr drp = servers_[i];
    IDriverSPtr curDr = drp->driver();
    if (curDr->settings() == settings) {
      return drp;
    }
  }
  return IServerSPtr();
}

std::vector<QObject *> ServersManager::findAllListeners(const IDriverSPtr &drv) const {
  std::vector<QObject *> result;
  for (size_t j = 0; j < servers_.size(); ++j) {
    IServerSPtr ser = servers_[j];
    if (ser->driver() == drv) {
      result.push_back(ser.get());
    }
  }
  return result;
}

}  // namespace fastonosql
