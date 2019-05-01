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

#include "proxy/db/redis/sentinel_settings.h"

#include <fastonosql/core/connection_types.h>

namespace fastonosql {
namespace proxy {
namespace redis {

SentinelSettings::SentinelSettings(const connection_path_t& connection_path)
    : ISentinelSettingsBase(connection_path, core::REDIS) {}

SentinelSettings* SentinelSettings::Clone() const {
  return new SentinelSettings(*this);
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
