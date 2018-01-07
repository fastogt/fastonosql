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

#pragma once

#include <common/patterns/singleton_pattern.h>  // for LazySingleton

#include <common/error.h>

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc
#include "proxy/connection_settings/isentinel_connection_settings.h"

#include "core/server/iserver_info.h"
#include "proxy/proxy_fwd.h"  // for IClusterSPtr, ISentinelSPtr, etc

namespace fastonosql {
namespace proxy {

class ServersManager : public common::patterns::LazySingleton<ServersManager> {
  friend class common::patterns::LazySingleton<ServersManager>;

 public:
  typedef IServerSPtr server_t;
  typedef IClusterSPtr cluster_t;
  typedef ISentinelSPtr sentinel_t;
  typedef std::vector<server_t> servers_t;

  server_t CreateServer(IConnectionSettingsBaseSPtr settings);
  sentinel_t CreateSentinel(ISentinelSettingsBaseSPtr settings);
  cluster_t CreateCluster(IClusterSettingsBaseSPtr settings);

  common::Error TestConnection(IConnectionSettingsBaseSPtr connection) WARN_UNUSED_RESULT;
  common::Error DiscoveryClusterConnection(IConnectionSettingsBaseSPtr connection,
                                           std::vector<core::ServerDiscoveryClusterInfoSPtr>* inf) WARN_UNUSED_RESULT;
  common::Error DiscoverySentinelConnection(IConnectionSettingsBaseSPtr connection,
                                            std::vector<core::ServerDiscoverySentinelInfoSPtr>* inf) WARN_UNUSED_RESULT;

  void Clear();

  void CloseServer(server_t server);
  void CloseCluster(cluster_t cluster);
  void CloseSentinel(sentinel_t sentinel);

 private:
  ServersManager();

  servers_t servers_;
};

}  // namespace proxy
}  // namespace fastonosql
