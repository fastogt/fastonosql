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

StatisticSender::StatisticSender(const std::string& login, uint32_t exec_count, QObject* parent)
    : QObject(parent), exec_count_(exec_count), login_(login) {}

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

  std::string request;
  common::Error request_gen_err = server::GenStatisticRequest(login_, exec_count_, &request);
  if (request_gen_err) {
    emit statisticSended(false);
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return;
  }

  size_t nwrite = 0;
  err = client.Write(request, &nwrite);
  if (err) {
    emit statisticSended(false);
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return;
  }

  std::string stat_reply;
  size_t nread = 0;
  err = client.Read(&stat_reply, 256, &nread);
  if (err) {
    emit statisticSended(false);
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return;
  }

  server::JsonRPCError jerror = server::ParseSendStatisticResponce(stat_reply);
  emit statisticSended(!jerror);
  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();
}

}  // namespace gui
}  // namespace fastonosql
