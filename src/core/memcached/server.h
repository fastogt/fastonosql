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

#include "common/net/types.h"  // for HostAndPort

#include "core/connection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/connection_types.h"     // for serverMode, serverState, etc
#include "core/core_fwd.h"             // for IDatabaseSPtr
#include "core/iserver.h"              // for IServerRemote
#include "core/types.h"                // for IDataBaseInfoSPtr

namespace fastonosql {
namespace core {
namespace memcached {

class Server : public IServerRemote {
  Q_OBJECT
 public:
  explicit Server(IConnectionSettingsBaseSPtr settings);
  virtual serverTypes role() const;
  virtual serverMode mode() const;
  virtual serverState state() const;
  virtual common::net::HostAndPort host() const;

 private:
  virtual IDatabaseSPtr createDatabase(IDataBaseInfoSPtr info);
};

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
