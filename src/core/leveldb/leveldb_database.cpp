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

#include "core/leveldb/leveldb_database.h"

#include "core/iserver.h"

namespace fastonosql {
namespace core {
namespace leveldb {

LeveldbDataBaseInfo::LeveldbDataBaseInfo(const std::string& name, bool isDefault,
                                         size_t size, const keys_container_t &keys)
  : IDataBaseInfo(name, isDefault, LEVELDB, size, keys) {
}

LeveldbDataBaseInfo* LeveldbDataBaseInfo::clone() const {
  return new LeveldbDataBaseInfo(*this);
}

LeveldbDatabase::LeveldbDatabase(IServerSPtr server, IDataBaseInfoSPtr info)
  : IDatabase(server, info) {
  DCHECK(server);
  DCHECK(info);
  DCHECK(server->type() == LEVELDB && info->type() == LEVELDB);
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
