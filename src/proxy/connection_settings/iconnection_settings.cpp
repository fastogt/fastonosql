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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/connection_settings/iconnection_settings.h"

#include <string>

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/utils.h>           // for decode64, encode64, crc64

#if defined(BUILD_WITH_REDIS)
#define LOGGING_REDIS_FILE_EXTENSION ".red"
#endif
#if defined(BUILD_WITH_MEMCACHED)
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#endif
#if defined(BUILD_WITH_SSDB)
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#endif
#if defined(BUILD_WITH_LEVELDB)
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#endif
#if defined(BUILD_WITH_ROCKSDB)
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#endif
#if defined(BUILD_WITH_UNQLITE)
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#endif
#if defined(BUILD_WITH_LMDB)
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"
#endif
#if defined(BUILD_WITH_UPSCALEDB)
#define LOGGING_UPSCALEDB_FILE_EXTENSION ".upscaledb"
#endif
#if defined(BUILD_WITH_FORESTDB)
#define LOGGING_FORESTDB_FILE_EXTENSION ".forestdb"
#endif
#if defined(BUILD_WITH_PIKA)
#define LOGGING_PIKA_FILE_EXTENSION ".pika"
#endif
#if defined(BUILD_WITH_DYNOMITE)
#define LOGGING_DYNOMITE_REDIS_FILE_EXTENSION ".dynred"
#endif

namespace fastonosql {
namespace {
const char* GetLoggingFileExtensionByConnectionType(core::ConnectionType type) {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    return LOGGING_REDIS_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    return LOGGING_MEMCACHED_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    return LOGGING_SSDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    return LOGGING_LEVELDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    return LOGGING_ROCKSDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    return LOGGING_UNQLITE_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    return LOGGING_LMDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (type == core::UPSCALEDB) {
    return LOGGING_UPSCALEDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    return LOGGING_FORESTDB_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    return LOGGING_PIKA_FILE_EXTENSION;
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (type == core::DYNOMITE) {
    return LOGGING_DYNOMITE_REDIS_FILE_EXTENSION;
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}
}  // namespace
namespace proxy {

const char IConnectionSettings::default_ns_separator[] = ":";

IConnectionSettings::IConnectionSettings(const connection_path_t& connection_path, core::ConnectionType type)
    : connection_path_(connection_path), type_(type), msinterval_(0) {}

void IConnectionSettings::SetPath(const connection_path_t& path) {
  connection_path_ = path;
}

connection_path_t IConnectionSettings::GetPath() const {
  return connection_path_;
}

core::ConnectionType IConnectionSettings::GetType() const {
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

IConnectionSettingsBase::IConnectionSettingsBase(const connection_path_t& connection_path,
                                                 const std::string& log_directory,
                                                 core::ConnectionType type)
    : IConnectionSettings(connection_path, type),
      log_directory_(common::file_system::stable_dir_path(log_directory)),
      hash_(),
      ns_separator_(default_ns_separator),
      ns_display_strategy_(FULL_KEY) {
  SetConnectionPathAndUpdateHash(connection_path);
}

NsDisplayStrategy IConnectionSettingsBase::GetNsDisplayStrategy() const {
  return ns_display_strategy_;
}

void IConnectionSettingsBase::SetNsDisplayStrategy(NsDisplayStrategy strategy) {
  ns_display_strategy_ = strategy;
}

std::string IConnectionSettingsBase::GetNsSeparator() const {
  return ns_separator_;
}

void IConnectionSettingsBase::SetNsSeparator(const std::string& ns) {
  ns_separator_ = ns;
}

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
  const std::string logging_path = log_directory_ + GetHash();
  return logging_path + GetLoggingFileExtensionByConnectionType(type_);
}

void IConnectionSettingsBase::PrepareInGuiIfNeeded() {}

}  // namespace proxy
}  // namespace fastonosql
