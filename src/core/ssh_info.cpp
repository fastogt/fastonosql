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

#include "core/ssh_info.h"

#include <common/convert2string.h>
#include <common/file_system/types.h>  // for prepare_path

#define DEFAULT_SSH_PORT 22
#define DEFAULT_PUB_KEY_PATH "~/.ssh/id_rsa.pub"
#define DEFAULT_PRIVATE_KEY_PATH "~/.ssh/id_rsa"

#define HOST_FIELD "host"
#define USER_FIELD "user"
#define PASSWORD_FIELD "password"
#define PUBKEY_FIELD "public_key"
#define PRIVKEY_FIELD "private_key"
#define USE_PUBLICKEY_FIELD "use_public_key"
#define PASSPHRASE_FIELD "passphrase"
#define CURMETHOD_FIELD "current_method"
#define MARKER "\r\n"

namespace {
const std::string g_ssh_methods[] = {"0", "1", "2", "3"};
}

namespace common {

std::string ConvertToString(fastonosql::core::SSHInfo::SupportedAuthenticationMethods method) {
  if (method >= 0 && method < SIZEOFMASS(g_ssh_methods)) {
    return g_ssh_methods[method];
  }

  DNOTREACHED();
  return g_ssh_methods[0];
}

bool ConvertFromString(const std::string& from, fastonosql::core::SSHInfo::SupportedAuthenticationMethods* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(g_ssh_methods); ++i) {
    if (from == g_ssh_methods[i]) {
      *out = static_cast<fastonosql::core::SSHInfo::SupportedAuthenticationMethods>(i);
      return true;
    }
  }

  DNOTREACHED();
  return false;
}

}  // namespace common

namespace fastonosql {
namespace core {

PublicPrivate::PublicPrivate()
    : PublicPrivate(common::file_system::prepare_path(DEFAULT_PUB_KEY_PATH),
                    common::file_system::prepare_path(DEFAULT_PRIVATE_KEY_PATH),
                    true) {}

PublicPrivate::PublicPrivate(const std::string& public_key, const std::string& private_key, bool use_public_key)
    : public_key(public_key), private_key(private_key), use_public_key(use_public_key) {}

bool PublicPrivate::IsValid() const {
  return !private_key.empty();
}

SSHInfo::SSHInfo()
    : host(common::net::HostAndPort::CreateLocalHost(DEFAULT_SSH_PORT)),
      user_name(),
      key(),
      current_method(UNKNOWN),
      password_(),
      runtime_password_() {}

SSHInfo::SSHInfo(const common::net::HostAndPort& host,
                 const std::string& user_name,
                 const std::string& password,
                 const std::string& public_key,
                 const std::string& private_key,
                 bool use_public_key,
                 const std::string& passphrase,
                 SupportedAuthenticationMethods method)
    : host(host),
      user_name(user_name),
      passphrase(passphrase),
      key(public_key, private_key, use_public_key),
      current_method(method),
      password_(),
      runtime_password_() {
  SetPassword(password);
}

SSHInfo::SSHInfo(const std::string& text)
    : host(common::net::HostAndPort::CreateLocalHost(DEFAULT_SSH_PORT)),
      user_name(),
      passphrase(),
      key(),
      current_method(UNKNOWN),
      password_(),
      runtime_password_() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = text.find(MARKER, start)) != std::string::npos) {
    std::string line = text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    if (delem != std::string::npos) {
      std::string field = line.substr(0, delem);
      std::string value = line.substr(delem + 1);
      if (field == HOST_FIELD) {
        common::net::HostAndPort lhost;
        if (common::ConvertFromString(value, &lhost)) {
          host = lhost;
        }
      } else if (field == USER_FIELD) {
        user_name = value;
      } else if (field == PASSWORD_FIELD) {
        SetPassword(value);
      } else if (field == PUBKEY_FIELD) {
        key.public_key = value;
      } else if (field == PRIVKEY_FIELD) {
        key.private_key = value;
      } else if (field == USE_PUBLICKEY_FIELD) {
        bool val;
        if (common::ConvertFromString(value, &val)) {
          key.use_public_key = val;
        }
      } else if (field == PASSPHRASE_FIELD) {
        passphrase = value;
      } else if (field == CURMETHOD_FIELD) {
        SupportedAuthenticationMethods lcurrent_method;
        if (common::ConvertFromString(value, &lcurrent_method)) {
          current_method = lcurrent_method;
        }
      }
    }
    start = pos + sizeof(MARKER) - 1;
  }
}

std::string SSHInfo::ToString() const {
  return HOST_FIELD ":" + common::ConvertToString(host) + MARKER USER_FIELD ":" + user_name +
         MARKER PASSWORD_FIELD ":" + GetPassword() + MARKER PUBKEY_FIELD ":" + key.public_key +
         MARKER PRIVKEY_FIELD ":" + key.private_key + MARKER USE_PUBLICKEY_FIELD ":" +
         common::ConvertToString(key.use_public_key) + MARKER PASSPHRASE_FIELD ":" + passphrase +
         MARKER CURMETHOD_FIELD ":" + common::ConvertToString(current_method) + MARKER;
}

bool SSHInfo::IsValid() const {
  if (current_method == PASSWORD) {
    return host.IsValid() && !user_name.empty() && !password_.empty();
  } else if (current_method == PUBLICKEY) {
    return host.IsValid() && !user_name.empty() && key.IsValid();
  } else if (current_method == ASK_PASSWORD) {
    return host.IsValid() && !user_name.empty() && password_.empty();
  };

  return false;
}

SSHInfo::SupportedAuthenticationMethods SSHInfo::GetAuthMethod() const {
  return current_method;
}

std::string SSHInfo::GetRuntimePassword() const {
  return runtime_password_;
}

void SSHInfo::SetPassword(const std::string& password) {
  runtime_password_ = password;
  if (current_method == ASK_PASSWORD) {
    password_ = std::string();
  } else {
    password_ = password;
  }
}

bool SSHInfo::Equals(const SSHInfo& other) const {
  return host == other.host && GetPassword() == other.GetPassword() && passphrase == other.passphrase &&
         user_name == other.user_name && current_method == other.current_method && key == other.key;
}

std::string SSHInfo::GetPassword() const {
  return password_;
}

}  // namespace core
}  // namespace fastonosql
