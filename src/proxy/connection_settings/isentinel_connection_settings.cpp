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

#include "proxy/connection_settings/isentinel_connection_settings.h"

namespace fastonosql {
namespace proxy {

SentinelSettings::SentinelSettings() : sentinel(), sentinel_nodes() {}

ISentinelSettingsBase::ISentinelSettingsBase(const connection_path_t& connection_path, core::ConnectionType type)
    : IConnectionSettings(connection_path, type), sentinel_nodes_() {}

ISentinelSettingsBase::sentinel_connections_t ISentinelSettingsBase::GetSentinels() const {
  return sentinel_nodes_;
}

void ISentinelSettingsBase::AddSentinel(sentinel_connection_t sent) {
  sentinel_nodes_.push_back(sent);
}

}  // namespace proxy
}  // namespace fastonosql
