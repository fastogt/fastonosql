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

#pragma once

#include <vector>

#include <QObject>

#include "common/patterns/singleton_pattern.h"

#include "core/types.h"
#include "core/core_fwd.h"
#include "core/connection_settings.h"

namespace fastonosql {
namespace core {

class ServersManager
  : public QObject, public common::patterns::LazySingleton<ServersManager> {
  friend class common::patterns::LazySingleton<ServersManager>;
  Q_OBJECT
 public:
  typedef IServerSPtr server_t;
  typedef IClusterSPtr cluster_t;
  typedef ISentinelSPtr sentinel_t;
  typedef std::vector<server_t> servers_container_t;

  server_t createServer(IConnectionSettingsBaseSPtr settings);
  sentinel_t createSentinel(ISentinelSettingsBaseSPtr settings);
  cluster_t createCluster(IClusterSettingsBaseSPtr settings);

  common::Error testConnection(IConnectionSettingsBaseSPtr connection) WARN_UNUSED_RESULT;
  common::Error discoveryClusterConnection(IConnectionSettingsBaseSPtr connection,
                                    std::vector<ServerDiscoveryClusterInfoSPtr>* inf) WARN_UNUSED_RESULT;
  common::Error discoverySentinelConnection(IConnectionSettingsBaseSPtr connection,
                                            std::vector<ServerDiscoverySentinelInfoSPtr>* inf) WARN_UNUSED_RESULT;

  void clear();

 public Q_SLOTS:
  void closeServer(server_t server);
  void closeCluster(cluster_t cluster);

 private:
  ServersManager();

  servers_container_t servers_;
};

}  // namespace core
}  // namespace fastonosql
