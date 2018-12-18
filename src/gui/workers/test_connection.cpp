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

#include "gui/workers/test_connection.h"

#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/time.h>               // for current_mstime

#include "proxy/servers_manager.h"  // for ServersManager

namespace fastonosql {
namespace gui {

TestConnection::TestConnection(proxy::IConnectionSettingsBaseSPtr conn, QObject* parent)
    : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {}

common::time64_t TestConnection::elipsedTime() const {
  return common::time::current_mstime() - start_time_;
}

void TestConnection::routine() {
  if (!connection_) {
    emit connectionResult(false, elipsedTime(), "Invalid connection settings");
    return;
  }

  common::Error err = proxy::ServersManager::GetInstance().TestConnection(connection_);
  const common::time64_t msec_exec = elipsedTime();
  if (err) {
    QString qdesc;
    common::ConvertFromString(err->GetDescription(), &qdesc);
    emit connectionResult(false, msec_exec, qdesc);
  } else {
    emit connectionResult(true, msec_exec, QString());
  }
}

}  // namespace gui
}  // namespace fastonosql
