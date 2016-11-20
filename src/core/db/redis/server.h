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

#include <common/net/types.h>  // for HostAndPort

#include "core/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/connection_types.h"                          // for serverMode, serverTypes, etc
#include "core/core_fwd.h"                                  // for IDatabaseSPtr
#include "core/events/events.h"                             // for DiscoveryInfoResponceEvent
#include "core/server/iserver_remote.h"                     // for IServerRemote

namespace fastonosql {
namespace core {
namespace redis {

class Server : public IServerRemote {
  Q_OBJECT
 public:
  explicit Server(IConnectionSettingsBaseSPtr settings);
  virtual serverTypes Role() const override;
  virtual serverMode Mode() const override;
  virtual serverState State() const override;
  virtual common::net::HostAndPort Host() const override;

 protected:
  virtual void HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) override;

 private:
  virtual IDatabaseSPtr CreateDatabase(IDataBaseInfoSPtr info) override;
  serverTypes role_;
  serverMode mode_;
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
