/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "app/iverify_user.h"

#include <string>

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/qt/convert2string.h>

namespace fastonosql {

common::Error GenerateHash(const std::string& password, std::string* result) {
  if (password.empty() || !result) {
    return common::make_error_inval();
  }

  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_CTX ctx;
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, reinterpret_cast<const unsigned char*>(password.data()), password.size());
  common::hash::MD5_Final(&ctx, md5_result);
  std::string hexed_password;
  const std::string md5_data(md5_result, md5_result + MD5_HASH_LENGHT);
  if (!common::utils::hex::encode(md5_data, true, &hexed_password)) {
    return common::make_error_inval();
  }

  *result = hexed_password;
  return common::Error();
}

IVerifyUser::IVerifyUser(const QString& login,
                         const QString& password,
                         proxy::UserInfo::BuildStrategy build_strategy,
                         QObject* parent)
    : QObject(parent), login_(login), password_(password), build_strategy_(build_strategy) {}

IVerifyUser::~IVerifyUser() {}

common::Error IVerifyUser::startVerification(const QString& login,
                                             const QString& password,
                                             proxy::UserInfo::BuildStrategy strategy,
                                             proxy::UserInfo* user_info_out) {
  if (login.isEmpty() || password.isEmpty() || !user_info_out) {
    return common::make_error_inval();
  }

  const std::string login_str = common::ConvertToString(login.toLower());
  const std::string password_str = common::ConvertToString(password);
  std::string hexed_password;
  common::Error err = GenerateHash(password_str, &hexed_password);
  if (err) {
    return err;
  }
  return startVerificationImpl(login_str, hexed_password, strategy, user_info_out);
}

void IVerifyUser::routine() {
  proxy::UserInfo user_info_out;
  common::Error err = startVerification(login_, password_, build_strategy_, &user_info_out);
  emit verifyUserResult(err, user_info_out);
}

}  // namespace fastonosql
