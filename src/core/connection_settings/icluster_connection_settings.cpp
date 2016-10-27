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

#include "core/connection_settings/icluster_connection_settings.h"

#include <sstream>

#include <common/convert2string.h>

#include "core/connection_settings_factory.h"

namespace fastonosql {
namespace core {

IClusterSettingsBase::IClusterSettingsBase(const connection_path_t& connectionPath,
                                           connectionTypes type)
    : IConnectionSettings(connectionPath, type) {}

IClusterSettingsBase::cluster_nodes_t IClusterSettingsBase::Nodes() const {
  return clusters_nodes_;
}

void IClusterSettingsBase::AddNode(IConnectionSettingsBaseSPtr node) {
  if (!node) {
    DNOTREACHED();
    return;
  }

  clusters_nodes_.push_back(node);
}

std::string IClusterSettingsBase::ToString() const {
  std::stringstream str;
  str << IConnectionSettings::ToString() << ',';
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = clusters_nodes_[i];
    if (serv) {
      str << magicNumber << serv->ToString();
    }
  }

  std::string res = str.str();
  return res;
}

IConnectionSettingsBaseSPtr IClusterSettingsBase::FindSettingsByHost(
    const common::net::HostAndPort& host) const {
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr cur = clusters_nodes_[i];
    IConnectionSettingsRemote* remote = dynamic_cast<IConnectionSettingsRemote*>(cur.get());  // +
    CHECK(remote);
    if (remote->Host() == host) {
      return cur;
    }
  }

  return IConnectionSettingsBaseSPtr();
}

}  // namespace core
}  // namespace fastonosql
