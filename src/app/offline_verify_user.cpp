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

#include "app/offline_verify_user.h"

#include <string>

namespace fastonosql {

OfflineVerifyUser::OfflineVerifyUser(const QString& login,
                                     const QString& password,
                                     proxy::UserInfo::BuildStrategy build_strategy,
                                     QObject* parent)
    : base_class(login, password, build_strategy, parent) {}

common::Error OfflineVerifyUser::startVerificationImpl(const std::string& login,
                                                       const std::string& hexed_password,
                                                       proxy::UserInfo::BuildStrategy build_strategy,
                                                       proxy::UserInfo* user_info_out) {
  if (login != USER_LOGIN) {
    return common::make_error("Wrong login.");
  }

  if (hexed_password != USER_PASSWORD) {
    return common::make_error("Wrong password.");
  }

  fastonosql::proxy::UserInfo user_info(login, hexed_password, build_strategy);
  user_info.SetSubscriptionState(fastonosql::proxy::UserInfo::SUBSCRIBED);
  *user_info_out = user_info;
  return common::Error();
}

}  // namespace fastonosql
