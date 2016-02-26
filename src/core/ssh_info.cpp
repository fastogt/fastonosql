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

#include <string>

#include "common/convert2string.h"
#include "common/file_system.h"

#define HOST "host"
#define USER "user"
#define PASSWORD "password"
#define PUBKEY "publicKey"
#define PRIVKEY "privateKey"
#define PASSPHRASE "passphrase"
#define CURMETHOD "currentMethod"
#define MARKER "\r\n"

namespace fastonosql {

SSHInfo::SSHInfo()
  : host(DEFAULT_SSH_HOST, DEFAULT_SSH_PORT),
    user_name(), password(), public_key(common::file_system::prepare_path("~/.ssh/id_rsa.pub")),
    private_key(common::file_system::prepare_path("~/.ssh/id_rsa")), current_method(UNKNOWN) {
}

SSHInfo::SSHInfo(const common::net::hostAndPort &host, const std::string &userName,
                 const std::string &password, const std::string &publicKey,
                 const std::string &privateKey, const std::string &passphrase,
                 SupportedAuthenticationMetods method)
  : host(host), user_name(userName), password(password),
    public_key(publicKey), private_key(privateKey), passphrase(passphrase),
    current_method(method) {
}

SSHInfo::SSHInfo(const std::string& text)
  : host(DEFAULT_SSH_HOST, DEFAULT_SSH_PORT), user_name(), password(),
    public_key(common::file_system::prepare_path("~/.ssh/id_rsa.pub")),
    private_key(common::file_system::prepare_path("~/.ssh/id_rsa")), passphrase(),
    current_method(UNKNOWN) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = text.find(MARKER, start)) != std::string::npos) {
    std::string line = text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    if (delem != std::string::npos) {
      std::string field = line.substr(0, delem);
      std::string value = line.substr(delem + 1);
      if (field == HOST) {
        host = common::convertFromString<common::net::hostAndPort>(value);
      } else if (field == USER) {
        user_name = value;
      } else if (field == PASSWORD) {
        password = value;
      } else if (field == PUBKEY) {
        public_key = value;
      } else if (field == PRIVKEY) {
        private_key = value;
      } else if (field == PASSPHRASE) {
        passphrase = value;
      } else if (field == CURMETHOD) {
        current_method = (SupportedAuthenticationMetods)common::convertFromString<int>(value);
      }
    }
    start = pos + sizeof(MARKER) - 1;
  }
}

bool SSHInfo::isValid() const {
  return current_method != UNKNOWN;
}

SSHInfo::SupportedAuthenticationMetods SSHInfo::authMethod() const {
  return current_method;
}

}  // namespace fastonosql

namespace common {

std::string convertToString(const fastonosql::SSHInfo& ssh_info) {
  return  HOST":" + common::convertToString(ssh_info.host) + MARKER
          USER":" + ssh_info.user_name + MARKER
          PASSWORD":" + ssh_info.password + MARKER
          PUBKEY":" + ssh_info.public_key + MARKER
          PRIVKEY":" + ssh_info.private_key + MARKER
          PASSPHRASE":" + ssh_info.passphrase + MARKER
          CURMETHOD":" + common::convertToString(ssh_info.current_method) + MARKER;
}

template<>
fastonosql::SSHInfo convertFromString(const std::string& text) {
  return fastonosql::SSHInfo(text);
}

}  // namespace common
