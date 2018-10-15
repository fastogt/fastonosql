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

#include "proxy/cluster/icluster.h"

#include "proxy/server/iserver_remote.h"  // for IServerRemote

namespace fastonosql {
namespace proxy {

ICluster::ICluster(const std::string& name) : name_(name) {}

std::string ICluster::GetName() const {
  return name_;
}

ICluster::nodes_t ICluster::GetNodes() const {
  return nodes_;
}

void ICluster::AddServer(node_t serv) {
  VERIFY(QObject::connect(serv.get(), &IServer::RedirectRequested, this, &ICluster::RedirectRequest));
  nodes_.push_back(serv);
}

ICluster::node_t ICluster::GetRoot() const {
  for (auto node : nodes_) {
    IServerRemote* rserver = dynamic_cast<IServerRemote*>(node.get());  // +
    if (rserver && rserver->GetRole() == core::MASTER) {
      return node;
    }
  }

  DNOTREACHED();
  return node_t();
}

void ICluster::RedirectRequest(const common::net::HostAndPortAndSlot& host,
                               const events_info::ExecuteInfoRequest& req) {
  for (auto node : nodes_) {
    IServerRemote* rserver = dynamic_cast<IServerRemote*>(node.get());  // +
    if (!rserver) {
      continue;
    }

    common::net::HostAndPort server_host = rserver->GetHost();
    if (server_host == host) {
      proxy::events_info::ConnectInfoRequest connect_req(this);
      rserver->Connect(connect_req);
      events_info::ExecuteInfoRequest exec_req(req.initiator(), req.text, req.repeat, req.msec_repeat_interval,
                                               req.history, req.silence, req.logtype);
      rserver->Execute(exec_req);
      return;
    }
  }
}

}  // namespace proxy
}  // namespace fastonosql
