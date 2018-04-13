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

#include "proxy/user_info.h"

namespace fastonosql {
namespace proxy {

UserInfo::UserInfo()
    : login_(),
      password_(),
      first_name_(),
      last_name_(),
      subscription_state_(UNSUBSCIRBED),
      exec_count_(0),
      expire_time_(0),
      user_id_() {}

UserInfo::UserInfo(const std::string& login, const std::string& password)
    : login_(login),
      password_(password),
      first_name_(),
      last_name_(),
      subscription_state_(UNSUBSCIRBED),
      exec_count_(0),
      expire_time_(0),
      user_id_() {}

std::string UserInfo::GetFirstName() const {
  return first_name_;
}

void UserInfo::SetFirstName(const std::string& first_name) {
  first_name_ = first_name;
}

std::string UserInfo::GetLastName() const {
  return last_name_;
}

void UserInfo::SetLastName(const std::string& last_name) {
  last_name_ = last_name;
}

std::string UserInfo::GetLogin() const {
  return login_;
}

std::string UserInfo::GetPassword() const {
  return password_;
}

bool UserInfo::IsValid() const {
  return !login_.empty() && !password_.empty();
}

UserInfo::SubscriptionState UserInfo::GetSubscriptionState() const {
  return subscription_state_;
}

void UserInfo::SetSubscriptionState(SubscriptionState state) {
  subscription_state_ = state;
}

size_t UserInfo::GetExecCount() const {
  return exec_count_;
}

void UserInfo::SetExecCount(size_t count) {
  exec_count_ = count;
}

time_t UserInfo::GetExpireTime() const {
  return expire_time_;
}

void UserInfo::SetExpireTime(time_t expire_time) {
  expire_time_ = expire_time;
}

user_id_t UserInfo::GetUserID() const {
  return user_id_;
}

void UserInfo::SetUserID(user_id_t user_id) {
  user_id_ = user_id;
}

}  // namespace proxy
}  // namespace fastonosql
