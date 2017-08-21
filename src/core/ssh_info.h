/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/net/types.h>  // for operator==, HostAndPort

namespace fastonosql {
namespace core {

struct SSHInfo {
  enum SupportedAuthenticationMetods { UNKNOWN = 0, PASSWORD = 1, PUBLICKEY = 2 };

  SSHInfo();
  SSHInfo(const common::net::HostAndPort& host,
          const std::string& userName,
          const std::string& password,
          const std::string& publicKey,
          const std::string& privateKey,
          const std::string& passphrase,
          SupportedAuthenticationMetods method);

  explicit SSHInfo(const std::string& text);

  bool IsValid() const;
  SupportedAuthenticationMetods AuthMethod() const;

  common::net::HostAndPort host;
  std::string user_name;
  std::string password;
  std::string public_key;
  std::string private_key;
  std::string passphrase;

  SupportedAuthenticationMetods current_method;
};

inline bool operator==(const SSHInfo& r, const SSHInfo& l) {
  return r.host == l.host && r.password == l.password && r.public_key == l.public_key &&
         r.private_key == l.private_key && r.passphrase == l.passphrase && r.user_name == l.user_name;
}

}  // namespace core
}  // namespace fastonosql

namespace common {
std::string ConvertToString(const fastonosql::core::SSHInfo& ssh_info);
bool ConvertFromString(const std::string& from, fastonosql::core::SSHInfo* out);

std::string ConvertToString(fastonosql::core::SSHInfo::SupportedAuthenticationMetods method);
bool ConvertFromString(const std::string& from, fastonosql::core::SSHInfo::SupportedAuthenticationMetods* out);

}  // namespace common
