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

#include "core/cluster/icluster.h"

#include <common/macros.h>  // for DNOTREACHED

#include <memory>  // for shared_ptr
#include <string>  // for string

#include "core/connection_types.h"       // for serverTypes::MASTER
#include "core/server/iserver.h"         // for IServer
#include "core/server/iserver_remote.h"  // for IServerRemote

namespace fastonosql {
namespace core {

ICluster::ICluster(const std::string& name) : name_(name) {}

std::string ICluster::Name() const {
  return name_;
}

ICluster::nodes_t ICluster::Nodes() const {
  return nodes_;
}

void ICluster::AddServer(node_t serv) {
  nodes_.push_back(serv);
}

ICluster::node_t ICluster::Root() const {
  for (auto node : nodes_) {
    IServerRemote* rserver = dynamic_cast<IServerRemote*>(node.get());  // +
    if (rserver && rserver->Role() == MASTER) {
      return node;
    }
  }

  DNOTREACHED();
  return node_t();
}

}  // namespace core
}  // namespace fastonosql
