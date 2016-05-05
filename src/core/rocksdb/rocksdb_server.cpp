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

#include "core/rocksdb/rocksdb_server.h"

#include "core/rocksdb/rocksdb_driver.h"
#include "core/rocksdb/rocksdb_database.h"

namespace fastonosql {
namespace core {
namespace rocksdb {

RocksdbServer::RocksdbServer(IConnectionSettingsBaseSPtr settings)
  : IServerLocal(new RocksdbDriver(settings)) {
}

serverMode RocksdbServer::mode() const {
  return STANDALONE;
}

std::string RocksdbServer::path() const {
  RocksdbDriver* const ldrv = static_cast<RocksdbDriver* const>(drv_);
  return ldrv->path();
}

IDatabaseSPtr RocksdbServer::createDatabase(IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new RocksdbDatabase(shared_from_this(), info));
}

}  // namespace rocksdb
}  // namespace core {
}  // namespace fastonosql
