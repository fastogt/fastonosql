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

#include "proxy/connection_settings/iconnection_settings_ssh.h"

#include <common/qt/convert2string.h>

#include <QInputDialog>

namespace {
const QString trInputSSHPasswordForServer_1S = QObject::tr("SSH passoword for server: %1");
}

namespace fastonosql {
namespace proxy {

IConnectionSettingsRemoteSSH::IConnectionSettingsRemoteSSH(const connection_path_t& connection_path,
                                                           const std::string& log_directory,
                                                           core::ConnectionType type)
    : IConnectionSettingsRemote(connection_path, log_directory, type), ssh_info_() {}

core::SSHInfo IConnectionSettingsRemoteSSH::GetSSHInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemoteSSH::SetSSHInfo(const struct core::SSHInfo& info) {
  ssh_info_ = info;
}

void IConnectionSettingsRemoteSSH::PrepareInGuiIfNeeded() {
  if (ssh_info_.GetAuthMethod() != core::SSHInfo::ASK_PASSWORD) {
    return;
  }

  QString qserver_name;
  common::net::HostAndPort ssh_host = ssh_info_.GetHost();
  common::ConvertFromString(ssh_host.GetHost(), &qserver_name);
  bool ok;
  QString publish_text =
      QInputDialog::getText(nullptr, trInputSSHPasswordForServer_1S.arg(qserver_name), "Password:", QLineEdit::Password,
                            QString(), &ok, Qt::WindowCloseButtonHint);
  if (ok) {
    ssh_info_.SetPassword(common::ConvertToString(publish_text));
  }
}

}  // namespace proxy
}  // namespace fastonosql
