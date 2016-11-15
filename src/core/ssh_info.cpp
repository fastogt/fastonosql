/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <stddef.h>  // for size_t

#include <string>  // for allocator, string, etc

#include <common/convert2string.h>  // for ConvertFromString
#include <common/file_system.h>     // for prepare_path
#include <common/macros.h>          // for NOTREACHED, SIZEOFMASS

#define DEFAULT_SSH_PORT 22
#define DEFAULT_PUB_KEY_PATH "~/.ssh/id_rsa.pub"
#define DEFAULT_PRIVATE_KEY_PATH "~/.ssh/id_rsa"

#define HOST_FIELD "host"
#define USER_FIELD "user"
#define PASSWORD_FIELD "password"
#define PUBKEY_FIELD "publicKey"
#define PRIVKEY_FIELD "privateKey"
#define PASSPHRASE_FIELD "passphrase"
#define CURMETHOD_FIELD "currentMethod"
#define MARKER "\r\n"

namespace {
const std::string sshMethods[] = {"0", "1", "2"};
}

namespace common {

std::string ConvertToString(const fastonosql::core::SSHInfo& ssh_info) {
  return HOST_FIELD ":" + common::ConvertToString(ssh_info.host) + MARKER USER_FIELD ":" +
         ssh_info.user_name + MARKER PASSWORD_FIELD ":" + ssh_info.password +
         MARKER PUBKEY_FIELD ":" + ssh_info.public_key + MARKER PRIVKEY_FIELD ":" +
         ssh_info.private_key + MARKER PASSPHRASE_FIELD ":" + ssh_info.passphrase +
         MARKER CURMETHOD_FIELD ":" + common::ConvertToString(ssh_info.current_method) + MARKER;
}

template <>
fastonosql::core::SSHInfo ConvertFromString(const std::string& text) {
  return fastonosql::core::SSHInfo(text);
}

std::string ConvertToString(fastonosql::core::SSHInfo::SupportedAuthenticationMetods method) {
  return sshMethods[method];
}

template <>
fastonosql::core::SSHInfo::SupportedAuthenticationMetods ConvertFromString(
    const std::string& text) {
  for (size_t i = 0; i < SIZEOFMASS(sshMethods); ++i) {
    if (text == sshMethods[i]) {
      return static_cast<fastonosql::core::SSHInfo::SupportedAuthenticationMetods>(i);
    }
  }

  NOTREACHED();
  return fastonosql::core::SSHInfo::UNKNOWN;
}

}  // namespace common

namespace fastonosql {
namespace core {

SSHInfo::SSHInfo()
    : host(common::net::HostAndPort::createLocalHost(DEFAULT_SSH_PORT)),
      user_name(),
      password(),
      public_key(common::file_system::prepare_path(DEFAULT_PUB_KEY_PATH)),
      private_key(common::file_system::prepare_path(DEFAULT_PRIVATE_KEY_PATH)),
      current_method(UNKNOWN) {}

SSHInfo::SSHInfo(const common::net::HostAndPort& host,
                 const std::string& userName,
                 const std::string& password,
                 const std::string& publicKey,
                 const std::string& privateKey,
                 const std::string& passphrase,
                 SupportedAuthenticationMetods method)
    : host(host),
      user_name(userName),
      password(password),
      public_key(publicKey),
      private_key(privateKey),
      passphrase(passphrase),
      current_method(method) {}

SSHInfo::SSHInfo(const std::string& text)
    : host(common::net::HostAndPort::createLocalHost(DEFAULT_SSH_PORT)),
      user_name(),
      password(),
      public_key(common::file_system::prepare_path(DEFAULT_PUB_KEY_PATH)),
      private_key(common::file_system::prepare_path(DEFAULT_PRIVATE_KEY_PATH)),
      passphrase(),
      current_method(UNKNOWN) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = text.find(MARKER, start)) != std::string::npos) {
    std::string line = text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    if (delem != std::string::npos) {
      std::string field = line.substr(0, delem);
      std::string value = line.substr(delem + 1);
      if (field == HOST_FIELD) {
        host = common::ConvertFromString<common::net::HostAndPort>(value);
      } else if (field == USER_FIELD) {
        user_name = value;
      } else if (field == PASSWORD_FIELD) {
        password = value;
      } else if (field == PUBKEY_FIELD) {
        public_key = value;
      } else if (field == PRIVKEY_FIELD) {
        private_key = value;
      } else if (field == PASSPHRASE_FIELD) {
        passphrase = value;
      } else if (field == CURMETHOD_FIELD) {
        current_method = common::ConvertFromString<SupportedAuthenticationMetods>(value);
      }
    }
    start = pos + sizeof(MARKER) - 1;
  }
}

bool SSHInfo::IsValid() const {
  if (current_method == PASSWORD) {
    return host.isValid() && !user_name.empty() && !password.empty();
  } else if (current_method == PUBLICKEY) {
    return host.isValid() && !user_name.empty() && !private_key.empty();
  } else {
    return false;
  }
}

SSHInfo::SupportedAuthenticationMetods SSHInfo::AuthMethod() const {
  return current_method;
}

}  // namespace core
}  // namespace fastonosql
