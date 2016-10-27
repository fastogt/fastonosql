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

#define LOGGING_REDIS_FILE_EXTENSION ".red"
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"

namespace fastonosql {
namespace core {

ConnectionSettingsPath::ConnectionSettingsPath() : path_() {}

ConnectionSettingsPath::ConnectionSettingsPath(const std::string& path) : path_(path) {}

ConnectionSettingsPath::ConnectionSettingsPath(const common::file_system::ascii_string_path& path)
    : path_(path) {}

bool ConnectionSettingsPath::Equals(const ConnectionSettingsPath& path) const {
  return path_.equals(path.path_);
}

std::string ConnectionSettingsPath::Name() const {
  return path_.fileName();
}

std::string ConnectionSettingsPath::Directory() const {
  return path_.directory();
}

std::string ConnectionSettingsPath::ToString() const {
  return common::ConvertToString(path_);
}

ConnectionSettingsPath ConnectionSettingsPath::Root() {
  static common::file_system::ascii_string_path root(
      common::file_system::get_separator_string<char>());
  return ConnectionSettingsPath(root);
}

IConnectionSettings::IConnectionSettings(const connection_path_t& connectionPath,
                                         connectionTypes type)
    : connection_path_(connectionPath), type_(type), msinterval_(0) {}

IConnectionSettings::~IConnectionSettings() {}

void IConnectionSettings::SetPath(const connection_path_t& path) {
  connection_path_ = path;
}

connection_path_t IConnectionSettings::Path() const {
  return connection_path_;
}

connectionTypes IConnectionSettings::Type() const {
  return type_;
}

bool IConnectionSettings::IsLoggingEnabled() const {
  return msinterval_ != 0;
}

uint32_t IConnectionSettings::LoggingMsTimeInterval() const {
  return msinterval_;
}

void IConnectionSettings::SetLoggingMsTimeInterval(uint32_t mstime) {
  msinterval_ = mstime;
}

std::string IConnectionSettings::ToString() const {
  return common::MemSPrintf("%d,%s,%" PRIu32, type_, connection_path_.ToString(), msinterval_);
}

IConnectionSettingsBase::IConnectionSettingsBase(const connection_path_t& connectionPath,
                                                 connectionTypes type)
    : IConnectionSettings(connectionPath, type), hash_() {
  SetConnectionPathAndUpdateHash(connectionPath);
}

IConnectionSettingsBase::~IConnectionSettingsBase() {}

void IConnectionSettingsBase::SetConnectionPathAndUpdateHash(const connection_path_t& name) {
  SetPath(name);
  std::string path = connection_path_.ToString();
  common::buffer_t bcon = common::ConvertFromString<common::buffer_t>(path);
  uint64_t v = common::utils::hash::crc64(0, bcon);
  hash_ = common::ConvertToString(v);
}

std::string IConnectionSettingsBase::Hash() const {
  return hash_;
}

std::string IConnectionSettingsBase::LoggingPath() const {
  std::string logDir = common::ConvertToString(SettingsManager::instance().LoggingDirectory());
  std::string prefix = logDir + Hash();
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

std::string IConnectionSettingsBase::ToString() const {
  std::stringstream str;
  str << IConnectionSettings::ToString() << ',' << CommandLine();
  std::string res = str.str();
  return res;
}

}  // namespace core
}  // namespace fastonosql
