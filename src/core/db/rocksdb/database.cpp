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

#include "core/db/rocksdb/database.h"

#include <common/macros.h>  // for DCHECK

#include "core/connection_types.h"  // for connectionTypes::ROCKSDB

namespace fastonosql {
namespace core {
namespace rocksdb {

DataBaseInfo::DataBaseInfo(const std::string& name,
                           bool isDefault,
                           size_t size,
                           const keys_container_t& keys)
    : IDataBaseInfo(name, isDefault, ROCKSDB, size, keys) {}

DataBaseInfo* DataBaseInfo::Clone() const {
  return new DataBaseInfo(*this);
}

Database::Database(IServerSPtr server, IDataBaseInfoSPtr info) : IDatabase(server, info) {
  DCHECK(server);
  DCHECK(info);
  DCHECK(info->Type() == ROCKSDB);
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
