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

#include "proxy/proxy_fwd.h"  // for IDatabaseSPtr

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/connection_types.h"                          // for serverMode, serverState, etc
#include "proxy/server/iserver_remote.h"                     // for IServerRemote
#include "core/database/idatabase_info.h"                   // for IDataBaseInfoSPtr

namespace fastonosql {
namespace proxy {
namespace ssdb {

class Server : public IServerRemote {
  Q_OBJECT
 public:
  explicit Server(IConnectionSettingsBaseSPtr settings);
  virtual ~Server();

  virtual serverMode Mode() const override;
  virtual serverTypes Role() const override;
  virtual serverState State() const override;
  virtual common::net::HostAndPort Host() const override;

 private:
  virtual IDatabaseSPtr CreateDatabase(IDataBaseInfoSPtr info) override;
};

}  // namespace ssdb
}  // namespace proxy
}  // namespace fastonosql
