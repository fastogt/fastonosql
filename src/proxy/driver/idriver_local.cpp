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

#include "proxy/driver/idriver_local.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace fastonosql {
namespace proxy {

IDriverLocal::IDriverLocal(IConnectionSettingsBaseSPtr settings) : IDriver(settings) {
  CHECK(IsLocalType(GetType()));
}

std::string IDriverLocal::GetPath() const {
  auto local_settings = GetSpecificSettings<IConnectionSettingsLocal>();
  return local_settings->GetDBPath();
}

}  // namespace proxy
}  // namespace fastonosql
