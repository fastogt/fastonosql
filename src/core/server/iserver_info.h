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

#pragma once

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint32_t

#include <memory>  // for shared_ptr
#include <string>  // for string

#include <common/net/types.h>  // for HostAndPortAndSlot
#include <common/types.h>      // for time64_t
#include <common/smart_ptr.h>

#include "core/connection_types.h"  // for connectionTypes, etc

namespace common {
class Value;
}  // lines 34-34

namespace fastonosql {
namespace core {

struct ServerCommonInfo {
  ServerCommonInfo();
  ServerCommonInfo(const std::string& name,
                   serverTypes type,
                   serverState state,
                   serverConnectState cstate);

  std::string name;           // name
  serverTypes type;           // role
  serverState state;          // state
  serverConnectState cstate;  // conection state
  common::net::HostAndPortAndSlot host;
};

class ServerDiscoveryInfoBase {
 public:
  connectionTypes connectionType() const;
  ServerCommonInfo info() const;

  std::string name() const;
  void setName(const std::string& name);

  common::net::HostAndPortAndSlot host() const;
  void setHost(const common::net::HostAndPortAndSlot& host);

 protected:
  ServerDiscoveryInfoBase(connectionTypes ctype, const ServerCommonInfo& info);
  ~ServerDiscoveryInfoBase();

 private:
  const connectionTypes ctype_;
  ServerCommonInfo info_;
};

class ServerDiscoverySentinelInfo : public ServerDiscoveryInfoBase {
 public:
  virtual ~ServerDiscoverySentinelInfo();

 protected:
  ServerDiscoverySentinelInfo(connectionTypes ctype, const ServerCommonInfo& info);
};

typedef common::shared_ptr<ServerDiscoverySentinelInfo> ServerDiscoverySentinelInfoSPtr;

class ServerDiscoveryClusterInfo : public ServerDiscoveryInfoBase {
 public:
  bool self() const;
  virtual ~ServerDiscoveryClusterInfo();

 protected:
  ServerDiscoveryClusterInfo(connectionTypes ctype, const ServerCommonInfo& info, bool self);

 private:
  const bool self_;
};

typedef common::shared_ptr<ServerDiscoveryClusterInfo> ServerDiscoveryClusterInfoSPtr;

class IServerInfo {
 public:
  explicit IServerInfo(connectionTypes type);
  virtual ~IServerInfo();

  connectionTypes Type() const;
  virtual std::string ToString() const = 0;
  virtual uint32_t Version() const = 0;
  virtual common::Value* ValueByIndexes(unsigned char property, unsigned char field) const = 0;

 private:
  const connectionTypes type_;
};

typedef common::shared_ptr<IServerInfo> IServerInfoSPtr;

struct ServerInfoSnapShoot {
  ServerInfoSnapShoot();
  ServerInfoSnapShoot(common::time64_t msec, IServerInfoSPtr info);
  bool isValid() const;

  common::time64_t msec;
  IServerInfoSPtr info;
};

}  // namespace core
}  // namespace fastonosql
