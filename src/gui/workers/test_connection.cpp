/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "gui/workers/test_connection.h"

#include <common/time.h>

#include "proxy/servers_manager.h"

namespace fastonosql {
namespace gui {

TestConnection::TestConnection(proxy::IConnectionSettingsBaseSPtr conn, QObject* parent)
    : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {
  qRegisterMetaType<common::Error>("common::Error");
}

common::time64_t TestConnection::elipsedTime() const {
  return common::time::current_mstime() - start_time_;
}

void TestConnection::routine() {
  if (!connection_) {
    emit connectionResult(common::make_error_inval(), elipsedTime());
    return;
  }

  const common::Error err = proxy::ServersManager::GetInstance().TestConnection(connection_);
  const qint64 msec_exec = elipsedTime();
  emit connectionResult(err, msec_exec);
}

}  // namespace gui
}  // namespace fastonosql
