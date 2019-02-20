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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/workers/discovery_cluster_connection.h"

#include <common/qt/convert2string.h>
#include <common/time.h>

#include "proxy/servers_manager.h"

namespace fastonosql {
namespace gui {

DiscoveryConnection::DiscoveryConnection(proxy::IConnectionSettingsBaseSPtr conn, QObject* parent)
    : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {
  qRegisterMetaType<common::Error>("common::Error");
  qRegisterMetaType<std::vector<core::ServerDiscoveryClusterInfoSPtr>>(
      "std::vector<core::ServerDiscoveryClusterInfoSPtr>");
}

common::time64_t DiscoveryConnection::elipsedTime() const {
  return common::time::current_mstime() - start_time_;
}

void DiscoveryConnection::routine() {
  std::vector<core::ServerDiscoveryClusterInfoSPtr> inf;

  if (!connection_) {
    emit connectionResult(common::make_error_inval(), elipsedTime(), inf);
    return;
  }

  common::Error err = proxy::ServersManager::GetInstance().DiscoveryClusterConnection(connection_, &inf);
  emit connectionResult(err, elipsedTime(), inf);
}

}  // namespace gui
}  // namespace fastonosql
