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

#include "gui/dialogs/test_connection.h"

#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/time.h>               // for current_mstime

#include "proxy/servers_manager.h"  // for ServersManager

#include "translations/global.h"  // for trSuccess

namespace fastonosql {
namespace gui {

TestConnection::TestConnection(proxy::IConnectionSettingsBaseSPtr conn, QObject* parent)
    : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {}

void TestConnection::routine() {
  if (!connection_) {
    emit connectionResult(false, common::time::current_mstime() - start_time_, "Invalid connection settings");
    return;
  }

  common::Error err = proxy::ServersManager::Instance().TestConnection(connection_);
  if (err && err->IsError()) {
    QString qdesc;
    common::ConvertFromString(err->GetDescription(), &qdesc);
    emit connectionResult(false, common::time::current_mstime() - start_time_, qdesc);
  } else {
    emit connectionResult(true, common::time::current_mstime() - start_time_, translations::trSuccess);
  }
}

}  // namespace gui
}  // namespace fastonosql
