/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#pragma once

#include "proxy/connection_settings/iconnection_settings.h"
#include "proxy/server/iserver_remote.h"

namespace fastonosql {
namespace proxy {
namespace ssdb {

class Server : public IServerRemote {
  Q_OBJECT

 public:
  explicit Server(IConnectionSettingsBaseSPtr settings);
  ~Server() override;

  core::ServerMode GetMode() const override;
  core::ServerType GetRole() const override;
  core::ServerState GetState() const override;
  common::net::HostAndPort GetHost() const override;

 private:
  IDatabaseSPtr CreateDatabase(core::IDataBaseInfoSPtr info) override;
};

}  // namespace ssdb
}  // namespace proxy
}  // namespace fastonosql
