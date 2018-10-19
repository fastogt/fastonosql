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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/pika/cluster_settings.h"

#include <fastonosql/core/connection_types.h>  // for core::ConnectionType::REDIS

namespace fastonosql {
namespace proxy {
namespace pika {

ClusterSettings::ClusterSettings(const connection_path_t& connection_path)
    : IClusterSettingsBase(connection_path, core::PIKA) {}

ClusterSettings* ClusterSettings::Clone() const {
  return new ClusterSettings(*this);
}

}  // namespace pika
}  // namespace proxy
}  // namespace fastonosql
