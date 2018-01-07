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

#include "core/db_traits.h"

namespace fastonosql {
namespace core {

Field::Field(const std::string& name, common::Value::Type type) : name(name), type(type) {}

bool Field::IsIntegral() const {
  return common::Value::IsIntegral(type);
}

std::vector<common::Value::Type> GetSupportedValueTypes(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return DBTraits<REDIS>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return DBTraits<MEMCACHED>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return DBTraits<SSDB>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return DBTraits<LEVELDB>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return DBTraits<ROCKSDB>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return DBTraits<UNQLITE>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return DBTraits<LMDB>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == UPSCALEDB) {
    return DBTraits<UPSCALEDB>::GetSupportedValueTypes();
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == FORESTDB) {
    return DBTraits<FORESTDB>::GetSupportedValueTypes();
  }
#endif
  NOTREACHED();
  return std::vector<common::Value::Type>();
}

std::vector<info_field_t> GetInfoFieldsFromType(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return DBTraits<REDIS>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return DBTraits<MEMCACHED>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return DBTraits<SSDB>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return DBTraits<LEVELDB>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return DBTraits<ROCKSDB>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return DBTraits<UNQLITE>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return DBTraits<LMDB>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == UPSCALEDB) {
    return DBTraits<UPSCALEDB>::GetInfoFields();
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == FORESTDB) {
    return DBTraits<FORESTDB>::GetInfoFields();
  }
#endif
  NOTREACHED();
  return std::vector<info_field_t>();
}

}  // namespace core
}  // namespace fastonosql
