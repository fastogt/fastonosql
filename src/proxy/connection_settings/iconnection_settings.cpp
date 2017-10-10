/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/connection_settings/iconnection_settings.h"

#include <sstream>

#include <inttypes.h>  // for PRIu32

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/sprintf.h>
#include <common/utils.h>  // for decode64, encode64, crc64

#include <common/qt/convert2string.h>  // for ConvertToString

#include "proxy/settings_manager.h"

#ifdef BUILD_WITH_REDIS
#define LOGGING_REDIS_FILE_EXTENSION ".red"
#endif
#ifdef BUILD_WITH_MEMCACHED
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#endif
#ifdef BUILD_WITH_SSDB
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#endif
#ifdef BUILD_WITH_LEVELDB
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#endif
#ifdef BUILD_WITH_ROCKSDB
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#endif
#ifdef BUILD_WITH_UNQLITE
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#endif
#ifdef BUILD_WITH_LMDB
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"
#endif
#ifdef BUILD_WITH_UPSCALEDB
#define LOGGING_UPSCALEDB_FILE_EXTENSION ".upscaledb"
#endif
#ifdef BUILD_WITH_FORESTDB
#define LOGGING_FORESTDB_FILE_EXTENSION ".forestdb"
#endif

namespace fastonosql {
namespace proxy {

ConnectionSettingsPath::ConnectionSettingsPath() : path_() {}

ConnectionSettingsPath::ConnectionSettingsPath(const std::string& path) : path_(path) {}

ConnectionSettingsPath::ConnectionSettingsPath(const common::file_system::ascii_string_path& path) : path_(path) {}

bool ConnectionSettingsPath::Equals(const ConnectionSettingsPath& path) const {
  return path_.Equals(path.path_);
}

std::string ConnectionSettingsPath::GetName() const {
  std::string path = path_.GetPath();
  return common::file_system::get_file_or_dir_name(path);
}

std::string ConnectionSettingsPath::GetDirectory() const {
  return path_.GetDirectory();
}

std::string ConnectionSettingsPath::ToString() const {
  return common::ConvertToString(path_);
}

ConnectionSettingsPath ConnectionSettingsPath::GetRoot() {
  static common::file_system::ascii_string_path root(common::file_system::get_separator_string<char>());
  return ConnectionSettingsPath(root);
}

const char IConnectionSettings::default_ns_separator[] = ":";

IConnectionSettings::IConnectionSettings(const connection_path_t& connectionPath, core::connectionTypes type)
    : connection_path_(connectionPath), type_(type), msinterval_(0), ns_separator_(default_ns_separator) {}

IConnectionSettings::~IConnectionSettings() {}

void IConnectionSettings::SetPath(const connection_path_t& path) {
  connection_path_ = path;
}

connection_path_t IConnectionSettings::GetPath() const {
  return connection_path_;
}

core::connectionTypes IConnectionSettings::GetType() const {
  return type_;
}

bool IConnectionSettings::IsHistoryEnabled() const {
  return msinterval_ != 0;
}

int IConnectionSettings::GetLoggingMsTimeInterval() const {
  return msinterval_;
}

void IConnectionSettings::SetLoggingMsTimeInterval(int mstime) {
  msinterval_ = mstime;
}

std::string IConnectionSettings::GetNsSeparator() const {
  return ns_separator_;
}

void IConnectionSettings::SetNsSeparator(const std::string& ns) {
  ns_separator_ = ns;
}

std::string IConnectionSettings::ToString() const {
  std::basic_stringstream<char> wr;
  wr << type_ << setting_value_delemitr << connection_path_.ToString() << setting_value_delemitr << msinterval_
     << setting_value_delemitr << ns_separator_;
  return wr.str();
}

IConnectionSettingsBase::IConnectionSettingsBase(const connection_path_t& connectionPath, core::connectionTypes type)
    : IConnectionSettings(connectionPath, type), hash_() {
  SetConnectionPathAndUpdateHash(connectionPath);
}

IConnectionSettingsBase::~IConnectionSettingsBase() {}

void IConnectionSettingsBase::SetConnectionPathAndUpdateHash(const connection_path_t& name) {
  SetPath(name);
  std::string path = connection_path_.ToString();
  common::buffer_t bcon;
  if (common::ConvertFromString(path, &bcon)) {
    uint64_t v = common::utils::hash::crc64(0, bcon);
    hash_ = common::ConvertToString(v);
  }
}

std::string IConnectionSettingsBase::GetHash() const {
  return hash_;
}

std::string IConnectionSettingsBase::GetLoggingPath() const {
  const std::string logDir = common::ConvertToString(SettingsManager::GetInstance().LoggingDirectory());
  const std::string prefix = logDir + GetHash();
#ifdef BUILD_WITH_REDIS
  if (type_ == core::REDIS) {
    return prefix + LOGGING_REDIS_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type_ == core::MEMCACHED) {
    return prefix + LOGGING_MEMCACHED_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type_ == core::SSDB) {
    return prefix + LOGGING_SSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type_ == core::LEVELDB) {
    return prefix + LOGGING_LEVELDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type_ == core::ROCKSDB) {
    return prefix + LOGGING_ROCKSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type_ == core::UNQLITE) {
    return prefix + LOGGING_UNQLITE_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type_ == core::LMDB) {
    return prefix + LOGGING_LMDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type_ == core::UPSCALEDB) {
    return prefix + LOGGING_UPSCALEDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type_ == core::FORESTDB) {
    return prefix + LOGGING_FORESTDB_FILE_EXTENSION;
  }
#endif

  NOTREACHED();
  return std::string();
}

std::string IConnectionSettingsBase::ToString() const {
  return IConnectionSettings::ToString() + setting_value_delemitr + GetCommandLine();
}

}  // namespace proxy
}  // namespace fastonosql
