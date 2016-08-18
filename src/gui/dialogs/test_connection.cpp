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

#include "gui/dialogs/test_connection.h"

#include <memory>                       // for __shared_ptr

#include "common/convert2string.h"      // for ConvertFromString
#include "common/error.h"               // for Error
#include "common/time.h"                // for current_mstime
#include "common/value.h"               // for ErrorValue

#include "core/servers_manager.h"       // for ServersManager

#include "translations/global.h"        // for trSuccess

namespace fastonosql {
namespace gui {

TestConnection::TestConnection(core::IConnectionSettingsBaseSPtr conn, QObject* parent)
  : QObject(parent), connection_(conn), start_time_(common::time::current_mstime()) {
}

void TestConnection::routine() {
  if (!connection_) {
    emit connectionResult(false, common::time::current_mstime() - start_time_,
                          "Invalid connection settings");
    return;
  }

  common::Error er = core::ServersManager::instance().testConnection(connection_);

  if (er && er->isError()) {
    emit connectionResult(false, common::time::current_mstime() - start_time_,
                          common::ConvertFromString<QString>(er->description()));
  } else {
    emit connectionResult(true, common::time::current_mstime() - start_time_, translations::trSuccess);
  }
}

}  // namespace gui
}  // namespace fastonosql
