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

#include <common/net/types.h>  // for operator==, HostAndPort

namespace fastonosql {
namespace core {

struct PublicPrivate {
  PublicPrivate();
  PublicPrivate(const std::string& public_key, const std::string& private_key, bool use_public_key);
  bool IsValid() const;

  std::string public_key;
  std::string private_key;
  bool use_public_key;
};

inline bool operator==(const PublicPrivate& r, const PublicPrivate& l) {
  return r.public_key == l.public_key && r.private_key == l.private_key && r.use_public_key == l.use_public_key;
}

inline bool operator!=(const PublicPrivate& r, const PublicPrivate& l) {
  return !(r == l);
}

struct SSHInfo {
  enum SupportedAuthenticationMethods { UNKNOWN = 0, PUBLICKEY = 1, ASK_PASSWORD = 2, PASSWORD = 3 };

  SSHInfo();
  SSHInfo(const common::net::HostAndPort& host,
          const std::string& user_name,
          const std::string& password,
          const std::string& public_key,
          const std::string& private_key,
          bool use_public_key,
          const std::string& passphrase,
          SupportedAuthenticationMethods method);

  explicit SSHInfo(const std::string& text);

  std::string ToString() const;

  bool IsValid() const;
  SupportedAuthenticationMethods GetAuthMethod() const;

  common::net::HostAndPort host;
  std::string user_name;
  std::string passphrase;
  PublicPrivate key;
  SupportedAuthenticationMethods current_method;

  std::string GetRuntimePassword() const;
  void SetPassword(const std::string& password);

  bool Equals(const SSHInfo& other) const;

 private:
  std::string GetPassword() const;

  std::string password_;
  std::string runtime_password_;
};

inline bool operator==(const SSHInfo& r, const SSHInfo& l) {
  return r.Equals(l);
}

inline bool operator!=(const SSHInfo& r, const SSHInfo& l) {
  return !(r == l);
}

}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::core::SSHInfo::SupportedAuthenticationMethods method);
bool ConvertFromString(const std::string& from, fastonosql::core::SSHInfo::SupportedAuthenticationMethods* out);

}  // namespace common
