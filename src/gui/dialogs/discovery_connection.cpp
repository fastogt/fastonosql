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

#include "gui/dialogs/discovery_connection.h"

#include <memory>  // for __shared_ptr

#include <common/error.h>              // for Error
#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/time.h>               // for current_mstime
#include <common/value.h>              // for ErrorValue

#include "proxy/servers_manager.h"  // for ServersManager

#include "translations/global.h"  // for trSuccess

namespace fastonosql {
namespace gui {

DiscoveryConnection::DiscoveryConnection(proxy::IConnectionSettingsBaseSPtr conn, QObject* parent)
    : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {
  qRegisterMetaType<std::vector<core::ServerDiscoveryClusterInfoSPtr>>(
      "std::vector<core::ServerDiscoveryClusterInfoSPtr>");
}

void DiscoveryConnection::routine() {
  std::vector<core::ServerDiscoveryClusterInfoSPtr> inf;

  if (!connection_) {
    emit connectionResult(false, common::time::current_mstime() - start_time_, "Invalid connection settings", inf);
    return;
  }

  common::Error er = proxy::ServersManager::Instance().DiscoveryClusterConnection(connection_, &inf);

  if (er && er->IsError()) {
    QString qdesc;
    common::ConvertFromString(er->Description(), &qdesc);
    emit connectionResult(false, common::time::current_mstime() - start_time_, qdesc, inf);
  } else {
    emit connectionResult(true, common::time::current_mstime() - start_time_, translations::trSuccess, inf);
  }
}

}  // namespace gui
}  // namespace fastonosql
