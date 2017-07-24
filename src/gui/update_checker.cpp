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

#include "gui/update_checker.h"

#include <sys/types.h>  // for ssize_t
#include <memory>       // for __shared_ptr

#include <common/error.h>              // for ErrnoError, ErrnoErrorValue
#include <common/macros.h>             // for DCHECK
#include <common/net/socket_tcp.h>     // for ClientSocketTcp
#include <common/net/types.h>          // for HostAndPort
#include <common/qt/convert2string.h>  // for ConvertFromString

#include "server_config_daemon/server_config.h"

namespace fastonosql {
namespace gui {

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent) {}

void UpdateChecker::routine() {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_URL, SERV_VERSION_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_URL, SERV_VERSION_PORT));
#else
#error please specify url and port of version information
#endif
  common::ErrnoError err = client.Connect();
  if (err && err->IsError()) {
    emit versionAvailibled(false, QString());
    return;
  }

  size_t nwrite = 0;
#if defined(FASTONOSQL)
  err = client.Write(GET_FASTONOSQL_VERSION, sizeof(GET_FASTONOSQL_VERSION) - 1, &nwrite);
#elif defined(FASTOREDIS)
  err = client.Write(GET_FASTOREDIS_VERSION, sizeof(GET_FASTOREDIS_VERSION) - 1, &nwrite);
#else
#error please specify request to get version information
#endif
  if (err && err->IsError()) {
    emit versionAvailibled(false, QString());
    DCHECK(!client.Close());
    return;
  }

  char version[128] = {0};
  size_t nread = 0;
  err = client.Read(version, sizeof(version), &nread);
  if (err && err->IsError()) {
    emit versionAvailibled(false, QString());
    DCHECK(!client.Close());
    return;
  }

  QString qvers;
  common::ConvertFromString(version, &qvers);
  emit versionAvailibled(true, qvers);
  DCHECK(!client.Close());
  return;
}

}  // namespace gui
}  // namespace fastonosql
