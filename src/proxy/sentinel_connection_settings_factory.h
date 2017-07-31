/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/connection_settings/isentinel_connection_settings.h"

namespace fastonosql {
namespace proxy {

class SentinelConnectionSettingsFactory : public common::patterns::LazySingleton<SentinelConnectionSettingsFactory> {
 public:
  friend class common::patterns::LazySingleton<SentinelConnectionSettingsFactory>;

  ISentinelSettingsBase* CreateFromType(core::connectionTypes type, const connection_path_t& conName);
  ISentinelSettingsBase* CreateFromString(const std::string& val);
};

}  // namespace proxy
}  // namespace fastonosql
