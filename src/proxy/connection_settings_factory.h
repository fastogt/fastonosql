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

#include <common/patterns/singleton_pattern.h>  // for LazySingleton

#include "proxy/connection_settings/iconnection_settings.h"
#include "proxy/connection_settings/iconnection_settings_remote.h"
#include "proxy/connection_settings/iconnection_settings_ssh.h"

namespace fastonosql {
namespace proxy {

class ConnectionSettingsFactory
    : public common::patterns::LazySingleton<ConnectionSettingsFactory> {
 public:
  friend class common::patterns::LazySingleton<ConnectionSettingsFactory>;

  IConnectionSettingsBase* CreateFromType(core::connectionTypes type, const connection_path_t& conName);
  IConnectionSettingsBase* CreateFromString(const std::string& val);

  IConnectionSettingsRemote* CreateFromType(core::connectionTypes type,
                                            const connection_path_t& conName,
                                            const common::net::HostAndPort& host);
};

}  // namespace proxy
}  // namespace fastonosql
