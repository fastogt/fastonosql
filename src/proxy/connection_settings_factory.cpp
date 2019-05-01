/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "proxy/connection_settings_factory.h"

#include <string>

#include <common/convert2string.h>

#if defined(BUILD_WITH_REDIS)
#include "proxy/db/redis/connection_settings.h"
#endif
#if defined(BUILD_WITH_MEMCACHED)
#include "proxy/db/memcached/connection_settings.h"
#endif
#if defined(BUILD_WITH_SSDB)
#include "proxy/db/ssdb/connection_settings.h"
#endif
#if defined(BUILD_WITH_LEVELDB)
#include "proxy/db/leveldb/connection_settings.h"
#endif
#if defined(BUILD_WITH_ROCKSDB)
#include "proxy/db/rocksdb/connection_settings.h"
#endif
#if defined(BUILD_WITH_UNQLITE)
#include "proxy/db/unqlite/connection_settings.h"
#endif
#if defined(BUILD_WITH_LMDB)
#include "proxy/db/lmdb/connection_settings.h"
#endif
#if defined(BUILD_WITH_FORESTDB)
#include "proxy/db/forestdb/connection_settings.h"
#endif
#if defined(BUILD_WITH_PIKA)
#include "proxy/db/pika/connection_settings.h"
#endif
#if defined(BUILD_WITH_DYNOMITE)
#include "proxy/db/dynomite/connection_settings.h"
#endif
#if defined(BUILD_WITH_KEYDB)
#include "proxy/db/keydb/connection_settings.h"
#endif

namespace fastonosql {
namespace proxy {

const char kMagicNumber = 0x1E;
const char kSettingValueDelemiter = 0x1F;

serialize_t ConnectionSettingsFactory::ConvertSettingsToString(IConnectionSettings* settings) {
  common::char_writer<512> wr;
  const connection_path_t path = settings->GetPath();
  wr << common::ConvertToCharBytes(static_cast<unsigned char>(settings->GetType())) << kSettingValueDelemiter
     << path.ToString() << kSettingValueDelemiter << common::ConvertToCharBytes(settings->GetLoggingMsTimeInterval());
  return wr.str();
}

serialize_t ConnectionSettingsFactory::ConvertSettingsToString(IConnectionSettingsBase* settings) {
  common::char_writer<512> wr;
  wr << ConvertSettingsToString(static_cast<IConnectionSettings*>(settings));
  wr << kSettingValueDelemiter << settings->GetNsSeparator() << kSettingValueDelemiter
     << common::ConvertToCharBytes(static_cast<unsigned char>(settings->GetNsDisplayStrategy()))
     << kSettingValueDelemiter << settings->GetCommandLine();
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
    return new redis::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    return new memcached::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    return new ssdb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    return new leveldb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    return new rocksdb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    return new unqlite::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    return new lmdb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    return new forestdb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    return new pika::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (type == core::DYNOMITE) {
    return new dynomite::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_KEYDB)
  if (type == core::KEYDB) {
    return new keydb::ConnectionSettings(connection_path, logging_dir_);
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
        if (common::ConvertFromBytes(element_text, &connection_type)) {
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
        if (common::ConvertFromBytes(element_text, &ms_time)) {
          result->SetLoggingMsTimeInterval(ms_time);
        }
      } else if (comma_count == 3) {
        const std::string ns_separator_str = common::ConvertToString(element_text);
        result->SetNsSeparator(ns_separator_str);
      } else if (comma_count == 4) {
        uint8_t ns_strategy;
        if (common::ConvertFromBytes(element_text, &ns_strategy)) {
          result->SetNsDisplayStrategy(static_cast<NsDisplayStrategy>(ns_strategy));
        }
        if (!IsCanSSHConnection(result->GetType())) {
          const std::string cmd_str = std::string(value.begin() + i + 1, value.end());
          result->SetCommandLine(cmd_str);
          break;
        }
      }
#if defined(BUILD_WITH_REDIS) || defined(BUILD_WITH_PIKA) || defined(BUILD_WITH_DYNOMITE) || defined(BUILD_WITH_KEYDB)
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
  CHECK(core::IsRemoteType(type)) << "Must be remote type: " << type;
  IConnectionSettingsRemote* remote = nullptr;
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    remote = new redis::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    remote = new memcached::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    remote = new ssdb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    remote = new pika::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (type == core::DYNOMITE) {
    remote = new dynomite::ConnectionSettings(connection_path, logging_dir_);
  }
#endif
#if defined(BUILD_WITH_KEYDB)
  if (type == core::KEYDB) {
    remote = new keydb::ConnectionSettings(connection_path, logging_dir_);
  }
#endif

  if (!remote) {
    DNOTREACHED() << "Unknown type: " << type;
  }
  remote->SetHost(host);
  return remote;
}

ConnectionSettingsFactory::ConnectionSettingsFactory() {}

std::string ConnectionSettingsFactory::GetLoggingDirectory() const {
  return logging_dir_;
}

void ConnectionSettingsFactory::SetLoggingDirectory(const std::string& dir) {
  logging_dir_ = dir;
}

}  // namespace proxy
}  // namespace fastonosql
