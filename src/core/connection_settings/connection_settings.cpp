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

#include "core/connection_settings/connection_settings.h"

#include <inttypes.h>  // for PRIu32
#include <sstream>     // for stringstream, basic_ostream, etc
#include <stddef.h>    // for size_t
#include <string>      // for string, allocator, etc

#include <common/macros.h>          // for NOTREACHED, CHECK, etc
#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/sprintf.h>         // for MemSPrintf
#include <common/types.h>           // for buffer_t
#include <common/utils.h>           // for decode64, encode64, crc64

#include <common/qt/convert2string.h>  // for ConvertToString

#include "core/settings_manager.h"

#ifdef BUILD_WITH_REDIS
#include "core/redis/connection_settings.h"  // for ConnectionSettings
#define LOGGING_REDIS_FILE_EXTENSION ".red"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/connection_settings.h"  // for ConnectionSettings
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#endif
#ifdef BUILD_WITH_SSDB
#include "core/ssdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/connection_settings.h"  // for ConnectionSettings
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#endif
#ifdef BUILD_WITH_LMDB
#include "core/lmdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"
#endif

namespace fastonosql {
namespace core {

ConnectionSettingsPath::ConnectionSettingsPath() : path_() {}

ConnectionSettingsPath::ConnectionSettingsPath(const std::string& path) : path_(path) {}

ConnectionSettingsPath::ConnectionSettingsPath(const common::file_system::ascii_string_path& path)
    : path_(path) {}

bool ConnectionSettingsPath::equals(const ConnectionSettingsPath& path) const {
  return path_.equals(path.path_);
}

std::string ConnectionSettingsPath::name() const {
  return path_.fileName();
}

std::string ConnectionSettingsPath::directory() const {
  return path_.directory();
}

std::string ConnectionSettingsPath::toString() const {
  return common::ConvertToString(path_);
}

ConnectionSettingsPath ConnectionSettingsPath::root() {
  static common::file_system::ascii_string_path root(
      common::file_system::get_separator_string<char>());
  return ConnectionSettingsPath(root);
}

IConnectionSettings::IConnectionSettings(const connection_path_t& connectionPath,
                                         connectionTypes type)
    : connection_path_(connectionPath), type_(type), msinterval_(0) {}

IConnectionSettings::~IConnectionSettings() {}

void IConnectionSettings::setPath(const connection_path_t& path) {
  connection_path_ = path;
}

IConnectionSettings::connection_path_t IConnectionSettings::path() const {
  return connection_path_;
}

connectionTypes IConnectionSettings::type() const {
  return type_;
}

bool IConnectionSettings::loggingEnabled() const {
  return msinterval_ != 0;
}

uint32_t IConnectionSettings::loggingMsTimeInterval() const {
  return msinterval_;
}

void IConnectionSettings::setLoggingMsTimeInterval(uint32_t mstime) {
  msinterval_ = mstime;
}

std::string IConnectionSettings::toString() const {
  return common::MemSPrintf("%d,%s,%" PRIu32, type_, connection_path_.toString(), msinterval_);
}

IConnectionSettingsBase::IConnectionSettingsBase(const connection_path_t& connectionPath,
                                                 connectionTypes type)
    : IConnectionSettings(connectionPath, type), hash_() {
  setConnectionPathAndUpdateHash(connectionPath);
}

IConnectionSettingsBase::~IConnectionSettingsBase() {}

void IConnectionSettingsBase::setConnectionPathAndUpdateHash(const connection_path_t& name) {
  setPath(name);
  std::string path = connection_path_.toString();
  common::buffer_t bcon = common::ConvertFromString<common::buffer_t>(path);
  uint64_t v = common::utils::hash::crc64(0, bcon);
  hash_ = common::ConvertToString(v);
}

std::string IConnectionSettingsBase::hash() const {
  return hash_;
}

std::string IConnectionSettingsBase::loggingPath() const {
  std::string logDir = common::ConvertToString(SettingsManager::instance().loggingDirectory());
  std::string prefix = logDir + hash();
#ifdef BUILD_WITH_REDIS
  if (type_ == REDIS) {
    return prefix + LOGGING_REDIS_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type_ == MEMCACHED) {
    return prefix + LOGGING_MEMCACHED_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type_ == SSDB) {
    return prefix + LOGGING_SSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type_ == LEVELDB) {
    return prefix + LOGGING_LEVELDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type_ == ROCKSDB) {
    return prefix + LOGGING_ROCKSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type_ == UNQLITE) {
    return prefix + LOGGING_UNQLITE_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type_ == LMDB) {
    return prefix + LOGGING_LMDB_FILE_EXTENSION;
  }
#endif

  NOTREACHED();
  return std::string();
}

IConnectionSettingsBase* IConnectionSettingsBase::createFromType(connectionTypes type,
                                                                 const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return new ssdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return new leveldb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return new rocksdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return new unqlite::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return new lmdb::ConnectionSettings(conName);
  }
#endif
  return nullptr;
}

IConnectionSettingsBase* IConnectionSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IConnectionSettingsBase* result = nullptr;
  size_t len = val.size();
  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        connectionTypes crT = static_cast<connectionTypes>(elText[0] - 48);
        result = createFromType(crT, connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->setConnectionPathAndUpdateHash(path);
      } else if (commaCount == 2) {
        uint32_t msTime = common::ConvertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);
        if (!isRemoteType(result->type())) {
          result->setCommandLine(val.substr(i + 1));
          break;
        }
      } else if (commaCount == 3) {
        result->setCommandLine(elText);
        if (IConnectionSettingsRemoteSSH* remote =
                dynamic_cast<IConnectionSettingsRemoteSSH*>(result)) {
          SSHInfo sinf(val.substr(i + 1));
          remote->setSshInfo(sinf);
        }
        break;
      }
      commaCount++;
      elText.clear();
    } else {
      elText += ch;
    }
  }
  return result;
}

std::string IConnectionSettingsBase::toString() const {
  std::stringstream str;
  str << IConnectionSettings::toString() << ',' << commandLine();
  std::string res = str.str();
  return res;
}

IConnectionSettingsLocal::IConnectionSettingsLocal(const connection_path_t& connectionPath,
                                                   connectionTypes type)
    : IConnectionSettingsBase(connectionPath, type) {
  CHECK(!isRemoteType(type));
}

////===================== IConnectionSettingsRemote
///=====================////

IConnectionSettingsRemote::IConnectionSettingsRemote(const connection_path_t& connectionPath,
                                                     connectionTypes type)
    : IConnectionSettingsBase(connectionPath, type) {
  CHECK(isRemoteType(type));
}

IConnectionSettingsRemote::~IConnectionSettingsRemote() {}

std::string IConnectionSettingsRemote::fullAddress() const {
  return common::ConvertToString(host());
}

IConnectionSettingsRemote* IConnectionSettingsRemote::createFromType(
    connectionTypes type,
    const connection_path_t& conName,
    const common::net::HostAndPort& host) {
  IConnectionSettingsRemote* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    remote = new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    remote = new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    remote = new ssdb::ConnectionSettings(conName);
  }
#endif
  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->setHost(host);
  return remote;
}

IConnectionSettingsRemoteSSH::IConnectionSettingsRemoteSSH(const connection_path_t& connectionName,
                                                           connectionTypes type)
    : IConnectionSettingsRemote(connectionName, type), ssh_info_() {}

IConnectionSettingsRemoteSSH* IConnectionSettingsRemoteSSH::createFromType(
    connectionTypes type,
    const connection_path_t& conName,
    const common::net::HostAndPort& host) {
  IConnectionSettingsRemoteSSH* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    remote = new redis::ConnectionSettings(conName);
  }
#endif
  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->setHost(host);
  return remote;
}

std::string IConnectionSettingsRemoteSSH::toString() const {
  std::stringstream str;
  str << IConnectionSettingsBase::toString() << ',' << common::ConvertToString(ssh_info_);
  std::string res = str.str();
  return res;
}

SSHInfo IConnectionSettingsRemoteSSH::sshInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemoteSSH::setSshInfo(const SSHInfo& info) {
  ssh_info_ = info;
}
}  // namespace core
}  // namespace fastonosql
