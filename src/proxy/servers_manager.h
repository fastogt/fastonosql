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

#include <vector>

#include <common/patterns/singleton_pattern.h>  // for LazySingleton

#include <common/error.h>

#include "proxy/connection_settings/iconnection_settings.h"

#if defined(PRO_VERSION)
#include <fastonosql/core/cluster/cluster_discovery_info.h>
#include <fastonosql/core/sentinel/sentinel_discovery_info.h>

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#endif

#include "proxy/proxy_fwd.h"

namespace fastonosql {
namespace proxy {

class ServersManager : public common::patterns::LazySingleton<ServersManager> {
  friend class common::patterns::LazySingleton<ServersManager>;

 public:
  typedef IServerSPtr server_t;
#if defined(PRO_VERSION)
  typedef IClusterSPtr cluster_t;
  typedef ISentinelSPtr sentinel_t;
#endif
  typedef std::vector<server_t> servers_t;

  server_t CreateServer(IConnectionSettingsBaseSPtr settings);
#if defined(PRO_VERSION)
  sentinel_t CreateSentinel(ISentinelSettingsBaseSPtr settings);
  cluster_t CreateCluster(IClusterSettingsBaseSPtr settings);
#endif

  common::Error TestConnection(IConnectionSettingsBaseSPtr connection) WARN_UNUSED_RESULT;
#if defined(PRO_VERSION)
  common::Error DiscoveryClusterConnection(IConnectionSettingsBaseSPtr connection,
                                           std::vector<core::ServerDiscoveryClusterInfoSPtr>* out) WARN_UNUSED_RESULT;
  common::Error DiscoverySentinelConnection(IConnectionSettingsBaseSPtr connection,
                                            std::vector<core::ServerDiscoverySentinelInfoSPtr>* out) WARN_UNUSED_RESULT;
#endif

  void Clear();

  void CloseServer(server_t server);
#if defined(PRO_VERSION)
  void CloseCluster(cluster_t cluster);
  void CloseSentinel(sentinel_t sentinel);
#endif

 private:
  ServersManager();

  servers_t servers_;
};

}  // namespace proxy
}  // namespace fastonosql
