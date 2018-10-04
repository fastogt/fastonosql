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

#include "proxy/connection_settings/icluster_connection_settings.h"

#include <stddef.h>  // for size_t

#include <common/macros.h>     // for DNOTREACHED
#include <common/net/types.h>  // for HostAndPort, operator==

#include "proxy/connection_settings/iconnection_settings_remote.h"

namespace fastonosql {
namespace proxy {

IClusterSettingsBase::IClusterSettingsBase(const connection_path_t& connectionPath, core::ConnectionTypes type)
    : IConnectionSettings(connectionPath, type) {}

IClusterSettingsBase::cluster_nodes_t IClusterSettingsBase::GetNodes() const {
  return clusters_nodes_;
}

void IClusterSettingsBase::AddNode(IConnectionSettingsBaseSPtr node) {
  if (!node) {
    // DNOTREACHED();
    return;
  }

  clusters_nodes_.push_back(node);
}

std::string IClusterSettingsBase::ToString() const {
  std::stringstream str;
  str << IConnectionSettings::ToString() << setting_value_delemitr;
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = clusters_nodes_[i];
    if (serv) {
      str << magic_number << serv->ToString();
    }
  }

  std::string res = str.str();
  return res;
}

IConnectionSettingsBaseSPtr IClusterSettingsBase::FindSettingsByHost(const common::net::HostAndPort& host) const {
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr cur = clusters_nodes_[i];
    IConnectionSettingsRemote* remote = dynamic_cast<IConnectionSettingsRemote*>(cur.get());  // +
    common::net::HostAndPort hs = remote->GetHost();
    if (hs == host) {
      return cur;
    }
  }

  return IConnectionSettingsBaseSPtr();
}

}  // namespace proxy
}  // namespace fastonosql
