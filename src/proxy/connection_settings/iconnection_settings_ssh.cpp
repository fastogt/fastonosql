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

#include "proxy/connection_settings/iconnection_settings_ssh.h"

#include <common/qt/convert2string.h>

#include <QInputDialog>

#include "translations/global.h"

namespace {
const QString trInputSSHPasswordForServer_1S = QObject::tr("SSH passoword for server: %1");
}

namespace fastonosql {
namespace proxy {

IConnectionSettingsRemoteSSH::IConnectionSettingsRemoteSSH(const connection_path_t& connectionName,
                                                           core::connectionTypes type)
    : IConnectionSettingsRemote(connectionName, type), ssh_info_() {}

std::string IConnectionSettingsRemoteSSH::ToString() const {
  return IConnectionSettingsBase::ToString() + setting_value_delemitr + ssh_info_.ToString();
}

core::SSHInfo IConnectionSettingsRemoteSSH::GetSSHInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemoteSSH::SetSSHInfo(const struct core::SSHInfo& info) {
  ssh_info_ = info;
}

void IConnectionSettingsRemoteSSH::PrepareInGuiIfNeeded() {
  if (ssh_info_.current_method != core::SSHInfo::ASK_PASSWORD) {
    return;
  }

  QString qserver_name;
  common::ConvertFromString(ssh_info_.host.GetHost(), &qserver_name);
  bool ok;
  QString publish_text = QInputDialog::getText(nullptr, trInputSSHPasswordForServer_1S.arg(qserver_name),
                                               fastonosql::translations::trPassword + ":", QLineEdit::Password,
                                               QString(), &ok, Qt::WindowCloseButtonHint);
  if (ok) {
    ssh_info_.SetPassword(common::ConvertToString(publish_text));
  }
}

}  // namespace proxy
}  // namespace fastonosql
