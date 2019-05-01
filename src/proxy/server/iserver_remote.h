/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "proxy/server/iserver.h"

namespace fastonosql {
namespace proxy {

class IServerRemote : public IServer {
  Q_OBJECT

 public:
  virtual common::net::HostAndPort GetHost() const = 0;
  virtual core::ServerMode GetMode() const = 0;
  virtual core::ServerType GetRole() const = 0;
  virtual core::ServerState GetState() const = 0;
  IDatabaseSPtr CreateDatabase(core::IDataBaseInfoSPtr info) override = 0;

 protected:
  explicit IServerRemote(IDriver* drv);
};

}  // namespace proxy
}  // namespace fastonosql
