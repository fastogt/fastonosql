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

#include "core/db/pika/db_connection.h"

#include <hiredis/hiredis.h>

namespace fastonosql {
namespace core {

template <>
const char* ConnectionTraits<PIKA>::GetBasedOn() {
  return "hiredis";
}

template <>
const char* ConnectionTraits<PIKA>::GetVersionApi() {
  return redis_compatible::GetHiredisVersion();
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<pika::NativeConnection, pika::RConfig>::Connect(const pika::RConfig& config,
                                                                                        pika::NativeConnection** hout) {
  pika::NativeConnection* context = nullptr;
  common::Error err = pika::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  // redisEnableKeepAlive(context);
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<pika::NativeConnection, pika::RConfig>::Disconnect(
    pika::NativeConnection** handle) {
  pika::NativeConnection* lhandle = *handle;
  if (lhandle) {
    redisFree(lhandle);
  }
  lhandle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<pika::NativeConnection, pika::RConfig>::IsConnected(pika::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const ConstantCommandsArray& CDBConnection<pika::NativeConnection, pika::RConfig, PIKA>::GetCommands() {
  static ConstantCommandsArray c{};
  return c;
}

}  // namespace internal

namespace pika {

common::Error CreateConnection(const RConfig& config, NativeConnection** context) {
  return redis_compatible::CreateConnection(config, config.ssh_info, context);
}

common::Error TestConnection(const RConfig& config) {
  return redis_compatible::TestConnection(config, config.ssh_info);
}

common::Error DiscoveryClusterConnection(const RConfig& config, std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  return redis_compatible::DiscoveryClusterConnection(config, config.ssh_info, infos);
}

common::Error DiscoverySentinelConnection(const RConfig& config, std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  return redis_compatible::DiscoverySentinelConnection(config, config.ssh_info, infos);
}

DBConnection::DBConnection(CDBConnectionClient* client) : base_class(client) {}

}  // namespace pika
}  // namespace core
}  // namespace fastonosql
