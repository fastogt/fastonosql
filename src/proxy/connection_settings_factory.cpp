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

#include "proxy/connection_settings_factory.h"

#include <string>

#include <common/convert2string.h>

#if defined(BUILD_WITH_REDIS)
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_MEMCACHED)
#include "proxy/db/memcached/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_SSDB)
#include "proxy/db/ssdb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_LEVELDB)
#include "proxy/db/leveldb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_ROCKSDB)
#include "proxy/db/rocksdb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_UNQLITE)
#include "proxy/db/unqlite/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_LMDB)
#include "proxy/db/lmdb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_UPSCALEDB)
#include "proxy/db/upscaledb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_FORESTDB)
#include "proxy/db/forestdb/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_PIKA)
#include "proxy/db/pika/connection_settings.h"  // for ConnectionSettings
#endif
#if defined(BUILD_WITH_DYNOMITE_REDIS)
#include "proxy/db/dynomite_redis/connection_settings.h"  // for ConnectionSettings
#endif

namespace fastonosql {
namespace proxy {

const char kMagicNumber = 0x1E;
const char kSettingValueDelemiter = 0x1F;

serialize_t ConnectionSettingsFactory::ConvertSettingsToString(IConnectionSettings* settings) {
  std::ostringstream wr;
  const connection_path_t path = settings->GetPath();
  wr << settings->GetType() << kSettingValueDelemiter << path.ToString() << kSettingValueDelemiter
     << settings->GetLoggingMsTimeInterval();
  return wr.str();
}

serialize_t ConnectionSettingsFactory::ConvertSettingsToString(IConnectionSettingsBase* settings) {
  std::ostringstream wr;
  wr << ConvertSettingsToString(static_cast<IConnectionSettings*>(settings));
  wr << kSettingValueDelemiter << settings->GetNsSeparator() << kSettingValueDelemiter
     << settings->GetNsDisplayStrategy() << kSettingValueDelemiter << settings->GetCommandLine();
  if (core::IsCanSSHConnection(settings->GetType())) {
    IConnectionSettingsRemoteSSH* ssh_settings = static_cast<IConnectionSettingsRemoteSSH*>(settings);
    wr << kSettingValueDelemiter << common::ConvertToString(ssh_settings->GetSSHInfo());
  }
  return wr.str();
}

IConnectionSettingsBase* ConnectionSettingsFactory::CreateSettingsFromTypeConnection(
    core::ConnectionType type,
    const connection_path_t& connection_path) {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    return CreateREDISConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    return CreateMEMCACHEDConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    return CreateSSDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    return CreateLEVELDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    return CreateROCKSDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    return CreateUNQLITEConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    return CreateLMDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (type == core::UPSCALEDB) {
    return CreateUPSCALEDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    return CreateFORESTDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    return CreatePIKAConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE_REDIS)
  if (type == core::DYNOMITE_REDIS) {
    return CreateDynomiteRedisConnection(connection_path);
  }
#endif

  NOTREACHED() << "Unknown type: " << type;
  return nullptr;
}

IConnectionSettingsBase* ConnectionSettingsFactory::CreateSettingsFromString(const serialize_t& value) {
  if (value.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  IConnectionSettingsBase* result = nullptr;
  size_t value_len = value.size();
  uint8_t comma_count = 0;
  serialize_t element_text;

  for (size_t i = 0; i < value_len; ++i) {
    serialize_t::value_type ch = value[i];
    if (ch == kSettingValueDelemiter) {
      if (comma_count == 0) {
        uint8_t connection_type;
        if (common::ConvertFromString(element_text, &connection_type)) {
          result =
              CreateSettingsFromTypeConnection(static_cast<core::ConnectionType>(connection_type), connection_path_t());
        }
        if (!result) {
          DNOTREACHED() << "Unknown connection_type: " << element_text;
          return nullptr;
        }
      } else if (comma_count == 1) {
        const std::string path_str = common::ConvertToString(element_text);
        connection_path_t path(path_str);
        result->SetConnectionPathAndUpdateHash(path);
      } else if (comma_count == 2) {
        int ms_time = 0;
        if (common::ConvertFromString(element_text, &ms_time)) {
          result->SetLoggingMsTimeInterval(ms_time);
        }
      } else if (comma_count == 3) {
        const std::string ns_separator_str = common::ConvertToString(element_text);
        result->SetNsSeparator(ns_separator_str);
      } else if (comma_count == 4) {
        uint8_t ns_strategy;
        if (common::ConvertFromString(element_text, &ns_strategy)) {
          result->SetNsDisplayStrategy(static_cast<NsDisplayStrategy>(ns_strategy));
        }
        if (!IsCanSSHConnection(result->GetType())) {
          const std::string cmd_str = std::string(value.begin() + i + 1, value.end());
          result->SetCommandLine(cmd_str);
          break;
        }
      }
#if defined(BUILD_WITH_REDIS) || defined(BUILD_WITH_PIKA) || defined(BUILD_WITH_DYNOMITE_REDIS)
      else if (comma_count == 5) {
        const std::string cmd_str = common::ConvertToString(element_text);
        result->SetCommandLine(cmd_str);
        if (core::IsCanSSHConnection(result->GetType())) {
          IConnectionSettingsRemoteSSH* remote = static_cast<IConnectionSettingsRemoteSSH*>(result);
          std::string ssh_str(value.begin() + i + 1, value.end());
          core::SSHInfo sinf;
          if (common::ConvertFromString(ssh_str, &sinf)) {
            remote->SetSSHInfo(sinf);
          }
        }
        break;
      }
#endif
      comma_count++;
      element_text.clear();
    } else {
      element_text.push_back(ch);
    }
  }

  DCHECK(result);
  return result;
}

IConnectionSettingsRemote* ConnectionSettingsFactory::CreateRemoteSettingsFromTypeConnection(
    core::ConnectionType type,
    const connection_path_t& connection_path,
    const common::net::HostAndPort& host) {
  IConnectionSettingsRemote* remote = nullptr;
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    remote = CreateREDISConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    remote = CreateMEMCACHEDConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    remote = CreateSSDBConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    remote = CreatePIKAConnection(connection_path);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE_REDIS)
  if (type == core::DYNOMITE_REDIS) {
    remote = CreateDynomiteRedisConnection(connection_path);
  }
#endif

  CHECK(remote) << "Unknown type: " << type;
  remote->SetHost(host);
  return remote;
}

ConnectionSettingsFactory::ConnectionSettingsFactory() {}

#if defined(BUILD_WITH_REDIS)
redis::ConnectionSettings* ConnectionSettingsFactory::CreateREDISConnection(
    const connection_path_t& connection_path) const {
  return new redis::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_MEMCACHED)
memcached::ConnectionSettings* ConnectionSettingsFactory::CreateMEMCACHEDConnection(
    const connection_path_t& connection_path) const {
  return new memcached::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_SSDB)
ssdb::ConnectionSettings* ConnectionSettingsFactory::CreateSSDBConnection(
    const connection_path_t& connection_path) const {
  return new ssdb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_LEVELDB)
leveldb::ConnectionSettings* ConnectionSettingsFactory::CreateLEVELDBConnection(
    const connection_path_t& connection_path) const {
  return new leveldb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_ROCKSDB)
rocksdb::ConnectionSettings* ConnectionSettingsFactory::CreateROCKSDBConnection(
    const connection_path_t& connection_path) const {
  return new rocksdb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_UNQLITE)
unqlite::ConnectionSettings* ConnectionSettingsFactory::CreateUNQLITEConnection(
    const connection_path_t& connection_path) const {
  return new unqlite::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_LMDB)
lmdb::ConnectionSettings* ConnectionSettingsFactory::CreateLMDBConnection(
    const connection_path_t& connection_path) const {
  return new lmdb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_UPSCALEDB)
upscaledb::ConnectionSettings* ConnectionSettingsFactory::CreateUPSCALEDBConnection(
    const connection_path_t& connection_path) const {
  return new upscaledb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_FORESTDB)
forestdb::ConnectionSettings* ConnectionSettingsFactory::CreateFORESTDBConnection(
    const connection_path_t& connection_path) const {
  return new forestdb::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_PIKA)
pika::ConnectionSettings* ConnectionSettingsFactory::CreatePIKAConnection(
    const connection_path_t& connection_path) const {
  return new pika::ConnectionSettings(connection_path, logging_dir_);
}
#endif

#if defined(BUILD_WITH_DYNOMITE_REDIS)
dynomite_redis::ConnectionSettings* ConnectionSettingsFactory::CreateDynomiteRedisConnection(
    const connection_path_t& connection_path) const {
  return new dynomite_redis::ConnectionSettings(connection_path, logging_dir_);
}
#endif

std::string ConnectionSettingsFactory::GetLoggingDirectory() const {
  return logging_dir_;
}

void ConnectionSettingsFactory::SetLoggingDirectory(const std::string& dir) {
  logging_dir_ = dir;
}

}  // namespace proxy
}  // namespace fastonosql
