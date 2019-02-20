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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <fastonosql/core/ssh_info.h>

#include "proxy/connection_settings/iconnection_settings_remote.h"

namespace fastonosql {
namespace proxy {

class IConnectionSettingsRemoteSSH : public IConnectionSettingsRemote {
 public:
  std::string GetDelimiter() const override = 0;
  void SetDelimiter(const std::string& delimiter) override = 0;

  common::net::HostAndPort GetHost() const override = 0;
  void SetHost(const common::net::HostAndPort& host) override = 0;

  std::string GetCommandLine() const override = 0;
  void SetCommandLine(const std::string& line) override = 0;

  struct core::SSHInfo GetSSHInfo() const;
  void SetSSHInfo(const struct core::SSHInfo& info);

  void PrepareInGuiIfNeeded() override;

 protected:
  IConnectionSettingsRemoteSSH(const connection_path_t& connection_path,
                               const std::string& log_directory,
                               core::ConnectionType type);

 private:
  struct core::SSHInfo ssh_info_;
};

}  // namespace proxy
}  // namespace fastonosql
