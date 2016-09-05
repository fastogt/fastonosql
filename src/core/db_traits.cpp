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

#include "core/db_traits.h"

#include "common/macros.h"  // for NOTREACHED

namespace fastonosql {
namespace core {

Field::Field(const std::string& name, common::Value::Type type) : name(name), type(type) {}

bool Field::isIntegral() const {
  return common::Value::isIntegral(type);
}

std::vector<common::Value::Type> supportedTypesFromType(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return DBTraits<REDIS>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return DBTraits<MEMCACHED>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return DBTraits<SSDB>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return DBTraits<LEVELDB>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return DBTraits<ROCKSDB>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return DBTraits<UNQLITE>::supportedTypes();
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return DBTraits<LMDB>::supportedTypes();
  }
#endif
  NOTREACHED();
  return std::vector<common::Value::Type>();
}

std::vector<info_field_t> infoFieldsFromType(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return DBTraits<REDIS>::infoFields();
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return DBTraits<MEMCACHED>::infoFields();
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return DBTraits<SSDB>::infoFields();
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return DBTraits<LEVELDB>::infoFields();
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return DBTraits<ROCKSDB>::infoFields();
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return DBTraits<UNQLITE>::infoFields();
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return DBTraits<LMDB>::infoFields();
  }
#endif
  NOTREACHED();
  return std::vector<info_field_t>();
}

}  // namespace core
}  // namespace fastonosql
