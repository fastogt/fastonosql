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

#include "gui/workers/statistic_sender.h"

#include <common/net/socket_tcp.h>  // for ClientSocketTcp
#include <common/qt/convert2string.h>

#include "proxy/server_config.h"  // for FASTONOSQL_URL, etc

namespace fastonosql {
namespace {
#if defined(PRO_VERSION)
common::Error sendStatisticRoutine(const std::string& login, const std::string& build_strategy) {
  CHECK(!login.empty());
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
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
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return request_gen_err;
  }

  size_t nwrite = 0;
  err = client.Write(request, &nwrite);
  if (err) {
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return common::make_error_from_errno(err);
  }

  std::string stat_reply;
  size_t nread = 0;
  err = client.Read(&stat_reply, 256, &nread);
  if (err) {
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return common::make_error_from_errno(err);
  }

  common::Error jerror = proxy::ParseSendStatisticResponce(stat_reply);
  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();

  return jerror;
}
#endif

common::Error sendAnonymousStatisticRoutine() {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
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
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return request_gen_err;
  }

  size_t nwrite = 0;
  err = client.Write(request, &nwrite);
  if (err) {
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return common::make_error_from_errno(err);
  }

  std::string stat_reply;
  size_t nread = 0;
  err = client.Read(&stat_reply, 256, &nread);
  if (err) {
    common::ErrnoError lerr = client.Close();
    DCHECK(!lerr) << "Close client error: " << err->GetDescription();
    return common::make_error_from_errno(err);
  }

  common::Error jerror = proxy::ParseSendStatisticResponce(stat_reply);
  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();

  return jerror;
}
}  // namespace

namespace gui {

AnonymousStatisticSender::AnonymousStatisticSender(QObject* parent) : QObject(parent) {}

void AnonymousStatisticSender::routine() {
  sendStatistic();
}

void AnonymousStatisticSender::sendStatistic() {
  common::Error err = sendAnonymousStatisticRoutine();
  if (err) {
    QString qerror_message;
    common::ConvertFromString(err->GetDescription(), &qerror_message);
    statisticSended(qerror_message);
    return;
  }

  statisticSended(QString());
}

#if defined(PRO_VERSION)
StatisticSender::StatisticSender(const std::string& login, const std::string& build_strategy, QObject* parent)
    : base_class(parent), login_(login), build_strategy_(build_strategy) {}

void StatisticSender::sendStatistic() {
  common::Error err = sendStatisticRoutine(login_, build_strategy_);
  if (err) {
    QString qerror_message;
    common::ConvertFromString(err->GetDescription(), &qerror_message);
    statisticSended(qerror_message);
    return;
  }

  statisticSended(QString());
}
#endif

}  // namespace gui
}  // namespace fastonosql
