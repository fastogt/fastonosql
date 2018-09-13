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

#pragma once

#include "core/db/redis_compatible/db_connection.h"

#include "core/db/pika/config.h"

namespace fastonosql {
namespace core {
namespace pika {

typedef redis_compatible::NativeConnection NativeConnection;

common::Error CreateConnection(const RConfig& config, NativeConnection** context);
common::Error TestConnection(const RConfig& config);
#if defined(PRO_VERSION)
common::Error DiscoveryClusterConnection(const RConfig& config, std::vector<ServerDiscoveryClusterInfoSPtr>* infos);
common::Error DiscoverySentinelConnection(const RConfig& config, std::vector<ServerDiscoverySentinelInfoSPtr>* infos);
#endif

class DBConnection : public redis_compatible::DBConnection<RConfig, PIKA> {
 public:
  typedef redis_compatible::DBConnection<RConfig, PIKA> base_class;
  explicit DBConnection(CDBConnectionClient* client);

 private:
  virtual common::Error DBkcountImpl(size_t* size) override;
};

}  // namespace pika
}  // namespace core
}  // namespace fastonosql
