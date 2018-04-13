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
  UserInfo();
  UserInfo(const std::string& login, const std::string& password);

  std::string GetFirstName() const;
  void SetFirstName(const std::string& first_name);

  std::string GetLastName() const;
  void SetLastName(const std::string& last_name);

  std::string GetLogin() const;
  std::string GetPassword() const;

  bool IsValid() const;

  SubscriptionState GetSubscriptionState() const;
  void SetSubscriptionState(SubscriptionState state);

  size_t GetExecCount() const;
  void SetExecCount(size_t count);

  time_t GetExpireTime() const;
  void SetExpireTime(time_t expire_time);

  user_id_t GetUserID() const;
  void SetUserID(user_id_t user_id);

 private:
  std::string login_;
  std::string password_;

  std::string first_name_;
  std::string last_name_;
  SubscriptionState subscription_state_;
  size_t exec_count_;
  time_t expire_time_;
  user_id_t user_id_;
};

}  // namespace proxy
}  // namespace fastonosql
