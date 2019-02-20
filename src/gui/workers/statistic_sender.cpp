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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/workers/statistic_sender.h"

#include <common/net/socket_tcp.h>

#include "proxy/server_config.h"

namespace fastonosql {
namespace {
#if defined(PRO_VERSION)
common::Error sendUserStatisticRoutine(const std::string& login, const std::string& build_strategy) {
  CHECK(!login.empty());
  typedef common::net::SocketGuard<common::net::ClientSocketTcp> ClientSocket;
#if defined(FASTONOSQL)
  ClientSocket client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  ClientSocket client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port to send statistic information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    return common::make_error_from_errno(err);
  }

  std::string request;
  common::Error request_gen_err = proxy::GenStatisticRequest(login, build_strategy, &request);
  if (request_gen_err) {
    return request_gen_err;
  }

  size_t nwrite = 0;
  err = client.Write(request.data(), request.size(), &nwrite);
  if (err) {
    return common::make_error_from_errno(err);
  }

  common::char_buffer_t stat_reply;
  err = client.ReadToBuffer(&stat_reply, 256);
  if (err) {
    return common::make_error_from_errno(err);
  }

  return proxy::ParseSendStatisticResponse(stat_reply.as_string());
}
#endif

common::Error sendAnonymousStatisticRoutine() {
  typedef common::net::SocketGuard<common::net::ClientSocketTcp> ClientSocket;
#if defined(FASTONOSQL)
  ClientSocket client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  ClientSocket client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port to send statistic information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    return common::make_error_from_errno(err);
  }

  std::string request;
  common::Error request_gen_err = proxy::GenAnonymousStatisticRequest(&request);
  if (request_gen_err) {
    return request_gen_err;
  }

  size_t nwrite = 0;
  err = client.Write(request.data(), request.size(), &nwrite);
  if (err) {
    return common::make_error_from_errno(err);
  }

  common::char_buffer_t stat_reply;
  err = client.ReadToBuffer(&stat_reply, 256);
  if (err) {
    return common::make_error_from_errno(err);
  }

  return proxy::ParseSendStatisticResponse(stat_reply.as_string());
}
}  // namespace

namespace gui {

AnonymousStatisticSender::AnonymousStatisticSender(QObject* parent) : QObject(parent) {
  qRegisterMetaType<common::Error>("common::Error");
}

void AnonymousStatisticSender::routine() {
  sendStatistic();
}

void AnonymousStatisticSender::sendStatistic() {
  const common::Error err = sendAnonymousStatisticRoutine();
  statisticSended(err);
}

#if defined(PRO_VERSION)
StatisticSender::StatisticSender(const std::string& login, const std::string& build_strategy, QObject* parent)
    : base_class(parent), login_(login), build_strategy_(build_strategy) {}

void StatisticSender::sendStatistic() {
  const common::Error err = sendUserStatisticRoutine(login_, build_strategy_);
  statisticSended(err);
}
#endif

}  // namespace gui
}  // namespace fastonosql
