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

#include "core/types.h"

#include <vector>
#include <string>

#include "common/string_util.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace core {

ServerCommonInfo::ServerCommonInfo()
  : name(), type(MASTER), state(SUP), cstate(SCONNECTED), host() {
}

ServerCommonInfo::ServerCommonInfo(const std::string& name, serverTypes type, serverState state, serverConnectState cstate)
  : name(name), type(type), state(state), cstate(cstate) {
}

ServerDiscoveryInfoBase::ServerDiscoveryInfoBase(connectionTypes ctype, const ServerCommonInfo &info)
  : ctype_(ctype), info_(info) {
}

connectionTypes ServerDiscoveryInfoBase::connectionType() const {
  return ctype_;
}

ServerCommonInfo ServerDiscoveryInfoBase::info() const {
  return info_;
}

std::string ServerDiscoveryInfoBase::name() const {
  return info_.name;
}

void ServerDiscoveryInfoBase::setName(const std::string& name) {
  info_.name = name;
}

common::net::HostAndPortAndSlot ServerDiscoveryInfoBase::host() const {
  return info_.host;
}

void ServerDiscoveryInfoBase::setHost(const common::net::HostAndPortAndSlot& host) {
  info_.host = host;
}

ServerDiscoveryInfoBase::~ServerDiscoveryInfoBase() {
}

ServerDiscoveryClusterInfo::ServerDiscoveryClusterInfo(connectionTypes ctype, const ServerCommonInfo& info, bool self)
  : ServerDiscoveryInfoBase(ctype, info), self_(self) {
}

ServerDiscoveryClusterInfo::~ServerDiscoveryClusterInfo() {
}

bool ServerDiscoveryClusterInfo::self() const {
  return self_;
}

IServerInfo::~IServerInfo() {
}

connectionTypes IServerInfo::type() const {
  return type_;
}

IServerInfo::IServerInfo(connectionTypes type)
  : type_(type) {
}

ServerDiscoverySentinelInfo::ServerDiscoverySentinelInfo(connectionTypes ctype, const ServerCommonInfo& info)
  : ServerDiscoveryInfoBase(ctype, info) {
}

ServerDiscoverySentinelInfo::~ServerDiscoverySentinelInfo() {
}

Field::Field(const std::string& name, common::Value::Type type)
  : name(name), type(type) {
}

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

ServerInfoSnapShoot::ServerInfoSnapShoot()
  : msec(0), info() {
}

ServerInfoSnapShoot::ServerInfoSnapShoot(common::time64_t msec, IServerInfoSPtr info)
  : msec(msec), info(info) {
}

bool ServerInfoSnapShoot::isValid() const {
  return msec > 0 && info;
}

IDataBaseInfo::IDataBaseInfo(const std::string& name, bool isDefault, connectionTypes type,
                           size_t dbkcount, const keys_container_t& keys)
  : name_(name), is_default_(isDefault), db_kcount_(dbkcount), keys_(keys), type_(type) {
}

IDataBaseInfo::~IDataBaseInfo() {
}

connectionTypes IDataBaseInfo::type() const {
  return type_;
}

std::string IDataBaseInfo::name() const {
  return name_;
}

size_t IDataBaseInfo::dbKeysCount() const {
  return db_kcount_;
}

void IDataBaseInfo::setDBKeysCount(size_t size) {
  db_kcount_ = size;
}

size_t IDataBaseInfo::loadedKeysCount() const {
  return keys_.size();
}

bool IDataBaseInfo::isDefault() const {
  return is_default_;
}

void IDataBaseInfo::setIsDefault(bool isDef) {
  is_default_ = isDef;
}

void IDataBaseInfo::setKeys(const keys_container_t& keys) {
  keys_ = keys;
}

void IDataBaseInfo::clearKeys() {
  keys_.clear();
}

IDataBaseInfo::keys_container_t IDataBaseInfo::keys() const {
  return keys_;
}

}  // namespace core
}  // namespace fastonosql
