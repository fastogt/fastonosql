/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "app/online_verify_user.h"

#include <string>

#include <common/net/socket_tcp.h>

#include "proxy/server_config.h"

namespace fastonosql {

OnlineVerifyUser::OnlineVerifyUser(const QString& login,
                                   const QString& password,
                                   proxy::UserInfo::BuildStrategy build_strategy,
                                   QObject* parent)
    : base_class(login, password, build_strategy, parent) {}

common::Error OnlineVerifyUser::startVerificationImpl(const std::string& login,
                                                      const std::string& hexed_password,
                                                      proxy::UserInfo::BuildStrategy strategy,
                                                      proxy::UserInfo* user_info_out) {
  typedef common::net::SocketGuard<common::net::ClientSocketTcp> ClientSocket;
#if defined(FASTONOSQL)
  ClientSocket client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  ClientSocket client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    return common::make_error("Sorry can't connect to server, for checking your credentials.");
  }

  proxy::UserInfo user_info(login, hexed_password, strategy);
  std::string request;
  common::Error request_err = proxy::GenSubscriptionStateRequest(user_info, &request);
  if (request_err) {
    return common::make_error("Sorry can't generate password request, for checking your credentials.");
  }

  size_t nwrite;
  err = client.Write(request.data(), request.size(), &nwrite);
  if (err) {
    return common::make_error("Sorry can't write request, for checking your credentials.");
  }

  common::char_buffer_t subscribe_reply;
  err = client.ReadToBuffer(&subscribe_reply, 256);
  if (err) {
    return common::make_error("Sorry can't get response, for checking your credentials.");
  }

  common::Error jerror = proxy::ParseSubscriptionStateResponse(subscribe_reply.as_string(), &user_info);
  if (jerror) {
    return jerror;
  }

  *user_info_out = user_info;
  return common::Error();
}

}  // namespace fastonosql
