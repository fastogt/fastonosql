/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/ssh_info.h"

#include "common/convert2string.h"
#include "common/file_system.h"

#define HOST "host"
#define PORT "port"
#define USER "user"
#define PASSWORD "password"
#define PUBKEY "publicKey"
#define PRIVKEY "privateKey"
#define PASSPHRASE "passphrase"
#define CURMETHOD "currentMethod"
#define MARKER "\r\n"

namespace fastonosql {

SSHInfo::SSHInfo()
  : hostName_(DEFAULT_SSH_HOST), port_(DEFAULT_SSH_PORT),
    userName_(), password_(), publicKey_(common::file_system::prepare_path("~/.ssh/id_rsa.pub")),
    privateKey_(common::file_system::prepare_path("~/.ssh/id_rsa")), currentMethod_(UNKNOWN) {
}

SSHInfo::SSHInfo(const std::string &hostName, int port, const std::string &userName, const std::string &password,
               const std::string &publicKey, const std::string &privateKey, const std::string &passphrase,
               SupportedAuthenticationMetods method)
  : hostName_(hostName), port_(port), userName_(userName), password_(password),
    publicKey_(publicKey), privateKey_(privateKey), passphrase_(passphrase), currentMethod_(method) {
}

SSHInfo::SSHInfo(const std::string& text)
  : hostName_(DEFAULT_SSH_HOST), port_(DEFAULT_SSH_PORT), userName_(), password_(),
    publicKey_(common::file_system::prepare_path("~/.ssh/id_rsa.pub")),
    privateKey_(common::file_system::prepare_path("~/.ssh/id_rsa")), passphrase_(),
    currentMethod_(UNKNOWN) {
  size_t pos = 0;
  size_t start = 0;
  while((pos = text.find(MARKER, start)) != std::string::npos){
      std::string line = text.substr(start, pos-start);
      size_t delem = line.find_first_of(':');
      std::string field = line.substr(0, delem);
      std::string value = line.substr(delem + 1);
      if (field == HOST) {
          hostName_ = value;
      } else if(field == PORT) {
          port_ = common::convertFromString<int>(value);
      } else if(field == USER) {
          userName_ = value;
      } else if(field == PASSWORD) {
          password_ = value;
      } else if(field == PUBKEY) {
          publicKey_ = value;
      } else if(field == PRIVKEY) {
          privateKey_ = value;
      } else if(field == PASSPHRASE) {
          passphrase_ = value;
      } else if(field == CURMETHOD) {
          currentMethod_ = (SupportedAuthenticationMetods)common::convertFromString<int>(value);
      }
      start = pos + 2;
  }
}

bool SSHInfo::isValid() const {
  return currentMethod_ != UNKNOWN;
}

SSHInfo::SupportedAuthenticationMetods SSHInfo::authMethod() const {
  return currentMethod_;
}

std::string SSHInfo::toString() const {
  return  HOST":" + hostName_  + MARKER
          PORT":" + common::convertToString(port_) + MARKER
          USER":" + userName_ + MARKER
          PASSWORD":" + password_ + MARKER
          PUBKEY":" + publicKey_ + MARKER
          PRIVKEY":" + privateKey_ + MARKER
          PASSPHRASE":" + passphrase_ + MARKER
          CURMETHOD":" + common::convertToString(currentMethod_) + MARKER;
}

}
