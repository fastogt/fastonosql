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

#include "gui/update_checker.h"

#include <common/net/socket_tcp.h>  // for ClientSocketTcp
#include <common/qt/convert2string.h>

#include "server/server_config.h"

namespace fastonosql {
namespace gui {

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent) {}

void UpdateChecker::routine() {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    emit versionAvailibled(false, QString());
    return;
  }

  std::string get_version_request;
  common::Error request_gen_err = server::GenVersionRequest(&get_version_request);
  if (request_gen_err) {
    emit versionAvailibled(false, QString());
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  size_t nwrite = 0;
  err = client.Write(get_version_request, &nwrite);
  if (err) {
    emit versionAvailibled(false, QString());
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  std::string version_reply;
  size_t nread = 0;
  err = client.Read(&version_reply, 256, &nread);
  if (err) {
    emit versionAvailibled(false, QString());
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  std::string version_str;
  server::JsonRPCError parse_error = server::ParseVersionResponce(version_reply, &version_str);
  if (parse_error) {
    emit versionAvailibled(false, QString());
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  QString qversion_str;
  common::ConvertFromString(version_str, &qversion_str);

  emit versionAvailibled(true, qversion_str);
  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();
}

}  // namespace gui
}  // namespace fastonosql
