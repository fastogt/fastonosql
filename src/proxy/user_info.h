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

#pragma once

#include <string>

namespace fastonosql {
namespace proxy {

typedef std::string user_id_t;

class UserInfo {
 public:
  enum SubscriptionState { UNSUBSCIRBED = 0, SUBSCRIBED };
  enum Type { USER = 0, SUPPORT = 1, OPEN_SOURCE = 2, ENTERPRISE = 3 };
  enum BuildStrategy { COMMUNITY_BUILD = 0, PUBLIC_BUILD = 1, PRIVATE_BUILD = 2 };
  UserInfo();
  UserInfo(const std::string& login, const std::string& password, BuildStrategy strategy);

  // only geters
  std::string GetLogin() const;
  std::string GetPassword() const;
  BuildStrategy GetBuildStrategy() const;

  bool IsValid() const;

  // server fields
  std::string GetFirstName() const;
  void SetFirstName(const std::string& first_name);

  std::string GetLastName() const;
  void SetLastName(const std::string& last_name);

  SubscriptionState GetSubscriptionState() const;
  void SetSubscriptionState(SubscriptionState state);

  Type GetType() const;
  void SetType(Type type);

  size_t GetExecCount() const;
  void SetExecCount(size_t count);

  time_t GetExpireTime() const;
  void SetExpireTime(time_t expire_time);

  user_id_t GetUserID() const;
  void SetUserID(user_id_t user_id);

 private:
  std::string login_;
  std::string password_;
  BuildStrategy build_strategy_;

  // server fields
  std::string first_name_;
  std::string last_name_;
  SubscriptionState subscription_state_;
  Type type_;
  size_t exec_count_;
  time_t expire_time_;
  user_id_t user_id_;
};

}  // namespace proxy
}  // namespace fastonosql

namespace common {
std::string ConvertToString(fastonosql::proxy::UserInfo::BuildStrategy t);
bool ConvertFromString(const std::string& from, fastonosql::proxy::UserInfo::BuildStrategy* out);

std::string ConvertToString(fastonosql::proxy::UserInfo::Type t);
bool ConvertFromString(const std::string& from, fastonosql::proxy::UserInfo::Type* out);
}  // namespace common
