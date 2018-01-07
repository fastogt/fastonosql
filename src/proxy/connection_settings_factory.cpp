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

#include <common/convert2string.h>

#ifdef BUILD_WITH_REDIS
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings
#define LOGGING_REDIS_FILE_EXTENSION ".red"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "proxy/db/memcached/connection_settings.h"  // for ConnectionSettings
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#endif
#ifdef BUILD_WITH_SSDB
#include "proxy/db/ssdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "proxy/db/leveldb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "proxy/db/rocksdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "proxy/db/unqlite/connection_settings.h"  // for ConnectionSettings
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#endif
#ifdef BUILD_WITH_LMDB
#include "proxy/db/lmdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"
#endif
#ifdef BUILD_WITH_UPSCALEDB
#include "proxy/db/upscaledb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_UPSCALEDB_FILE_EXTENSION ".upscaledb"
#endif
#ifdef BUILD_WITH_FORESTDB
#include "proxy/db/forestdb/connection_settings.h"  // for ConnectionSettings
#define LOGGING_FORESTDB_FILE_EXTENSION ".forestdb"
#endif

namespace fastonosql {
namespace proxy {

IConnectionSettingsBase* ConnectionSettingsFactory::CreateFromType(core::connectionTypes type,
                                                                   const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    return new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    return new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    return new ssdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    return new leveldb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    return new rocksdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    return new unqlite::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    return new lmdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    return new upscaledb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    return new forestdb::ConnectionSettings(conName);
  }
#endif
  return nullptr;
}

IConnectionSettingsBase* ConnectionSettingsFactory::CreateFromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IConnectionSettingsBase* result = nullptr;
  size_t len = val.size();
  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == setting_value_delemitr) {
      if (commaCount == 0) {
        core::connectionTypes crT = static_cast<core::connectionTypes>(elText[0] - 48);
        result = CreateFromType(crT, connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->SetConnectionPathAndUpdateHash(path);
      } else if (commaCount == 2) {
        int msTime;
        if (common::ConvertFromString(elText, &msTime)) {
          result->SetLoggingMsTimeInterval(msTime);
        }
      } else if (commaCount == 3) {
        result->SetNsSeparator(elText);
      } else if (commaCount == 4) {
        core::NsDisplayStrategy strat = static_cast<core::NsDisplayStrategy>(elText[0] - 48);
        result->SetNsDisplayStrategy(strat);
        if (!IsCanSSHConnection(result->GetType())) {
          result->SetCommandLine(val.substr(i + 1));
          break;
        }
      } else if (commaCount == 5) {
        result->SetCommandLine(elText);
        if (IConnectionSettingsRemoteSSH* remote = dynamic_cast<IConnectionSettingsRemoteSSH*>(result)) {
          core::SSHInfo sinf(val.substr(i + 1));
          remote->SetSSHInfo(sinf);
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

IConnectionSettingsRemote* ConnectionSettingsFactory::CreateFromType(core::connectionTypes type,
                                                                     const connection_path_t& conName,
                                                                     const common::net::HostAndPort& host) {
  IConnectionSettingsRemote* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    remote = new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    remote = new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    remote = new ssdb::ConnectionSettings(conName);
  }
#endif

  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->SetHost(host);
  return remote;
}

}  // namespace proxy
}  // namespace fastonosql
