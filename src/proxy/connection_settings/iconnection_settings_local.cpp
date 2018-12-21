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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace proxy {

IConnectionSettingsLocal::IConnectionSettingsLocal(const connection_path_t& connection_path,
                                                   const std::string& log_directory,
                                                   core::ConnectionType type)
    : IConnectionSettingsBase(connection_path, log_directory, type) {
  CHECK(IsLocalType(type));
}

std::string IConnectionSettingsLocal::GetFullAddress() const {
  return GetDBPath();
}

}  // namespace proxy
}  // namespace fastonosql
