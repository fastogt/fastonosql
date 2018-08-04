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

#include "verify_user.h"

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/net/socket_tcp.h>
#include <common/qt/convert2string.h>

#include "proxy/server_config.h"

namespace fastonosql {

namespace {
common::Error startVerification(const QString& login, const QString& password, proxy::UserInfo* uinf) {
  if (login.isEmpty() || password.isEmpty() || !uinf) {
    return common::make_error_inval();
  }

  const std::string login_str = common::ConvertToString(login.toLower());
  const std::string password_str = common::ConvertToString(password);
  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_CTX ctx;
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, reinterpret_cast<const unsigned char*>(password_str.data()), password_str.size());
  common::hash::MD5_Final(&ctx, md5_result);
  std::string hexed_password = common::utils::hex::encode(std::string(md5_result, md5_result + MD5_HASH_LENGHT), true);

#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    return common::make_error("Sorry can't connect to server, for checking your credentials.");
  }

  fastonosql::proxy::UserInfo user_info(login_str, hexed_password);
  std::string request;
  common::Error request_err = fastonosql::proxy::GenSubscriptionStateRequest(user_info, &request);
  if (request_err) {
    return common::make_error("Sorry can't generate password request, for checking your credentials.");
  }

  size_t nwrite;
  err = client.Write(request, &nwrite);
  if (err) {
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return common::make_error("Sorry can't write request, for checking your credentials.");
  }

  std::string subscribe_reply;
  size_t nread = 0;
  err = client.Read(&subscribe_reply, 256, &nread);
  if (err) {
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return common::make_error("Sorry can't get responce, for checking your credentials.");
  }

  common::Error jerror = fastonosql::proxy::ParseSubscriptionStateResponce(subscribe_reply, &user_info);
  if (jerror) {
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    return jerror;
  }

  *uinf = user_info;
  return common::Error();
}

}  // namespace

VerifyUser::VerifyUser(const QString& login, const QString& password, QObject* parent)
    : QObject(parent), login_(login), password_(password) {}

void VerifyUser::routine() {
  proxy::UserInfo uinf;
  common::Error err = startVerification(login_, password_, &uinf);
  emit verifyUserResult(err, uinf);
}

}  // namespace fastonosql
