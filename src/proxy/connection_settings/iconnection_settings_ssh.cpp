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

#include "proxy/connection_settings/iconnection_settings_ssh.h"

namespace fastonosql {
namespace proxy {

IConnectionSettingsRemoteSSH::IConnectionSettingsRemoteSSH(const connection_path_t& connectionName,
                                                           core::connectionTypes type)
    : IConnectionSettingsRemote(connectionName, type), ssh_info_() {}

std::string IConnectionSettingsRemoteSSH::ToString() const {
  return IConnectionSettingsBase::ToString() + setting_value_delemitr + common::ConvertToString(ssh_info_);
}

core::SSHInfo IConnectionSettingsRemoteSSH::GetSSHInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemoteSSH::SetSSHInfo(const struct core::SSHInfo& info) {
  ssh_info_ = info;
}

}  // namespace proxy
}  // namespace fastonosql
