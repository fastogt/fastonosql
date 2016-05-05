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

#include "core/lmdb/lmdb_server.h"

#include "core/lmdb/lmdb_driver.h"
#include "core/lmdb/lmdb_database.h"

namespace fastonosql {
namespace core {
namespace lmdb {

LmdbServer::LmdbServer(IConnectionSettingsBaseSPtr settings)
  : IServerLocal(new LmdbDriver(settings)) {
}

serverMode LmdbServer::mode() const {
  return STANDALONE;
}

std::string LmdbServer::path() const {
  LmdbDriver* const ldrv = static_cast<LmdbDriver* const>(drv_);
  return ldrv->path();
}

IDatabaseSPtr LmdbServer::createDatabase(IDataBaseInfoSPtr info) {
  return IDatabaseSPtr(new LmdbDatabase(shared_from_this(), info));
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
