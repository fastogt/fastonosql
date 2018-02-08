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

#include "gui/statistic_sender.h"

#include <common/net/socket_tcp.h>  // for ClientSocketTcp

#include "server/server_config.h"  // for FASTONOSQL_URL, etc

namespace fastonosql {
namespace gui {

StatisticSender::StatisticSender(uint32_t exec_count, QObject* parent) : QObject(parent), exec_count_(exec_count) {}

void StatisticSender::routine() {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port to send statistic information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    emit statisticSended(false);
    return;
  }

  size_t nwrite = 0;
  std::string request = server::SendStatisticRequest(exec_count_);
  err = client.Write(request, &nwrite);
  if (err) {
    emit statisticSended(false);
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  std::string stat_reply;
  size_t nread = 0;
  err = client.Read(&stat_reply, 256, &nread);
  if (err) {
    emit statisticSended(false);
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  bool is_sent;
  common::Error parse_error = server::ParseSendStatisticResponce(stat_reply, &is_sent);
  if (parse_error) {
    emit statisticSended(false);
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    return;
  }

  emit statisticSended(is_sent);
  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();
}

}  // namespace gui
}  // namespace fastonosql
