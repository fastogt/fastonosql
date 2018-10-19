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

#pragma once

#include <common/patterns/singleton_pattern.h>

#include "proxy/connection_settings/iconnection_settings.h"
#include "proxy/connection_settings/iconnection_settings_remote.h"
#include "proxy/connection_settings/iconnection_settings_ssh.h"

namespace fastonosql {
namespace proxy {

#ifdef BUILD_WITH_REDIS
namespace redis {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_MEMCACHED
namespace memcached {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_SSDB
namespace ssdb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_LEVELDB
namespace leveldb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_ROCKSDB
namespace rocksdb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_UNQLITE
namespace unqlite {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_LMDB
namespace lmdb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_UPSCALEDB
namespace upscaledb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_FORESTDB
namespace forestdb {
class ConnectionSettings;
}
#endif

#ifdef BUILD_WITH_PIKA
namespace pika {
class ConnectionSettings;
}
#endif

class ConnectionSettingsFactory : public common::patterns::LazySingleton<ConnectionSettingsFactory> {
 public:
  friend class common::patterns::LazySingleton<ConnectionSettingsFactory>;

  IConnectionSettingsBase* CreateFromTypeConnection(core::ConnectionType type,
                                                    const connection_path_t& connection_path);
  IConnectionSettingsBase* CreateFromStringConnection(const std::string& value);

  IConnectionSettingsRemote* CreateFromTypeConnection(core::ConnectionType type,
                                                      const connection_path_t& connection_path,
                                                      const common::net::HostAndPort& host);

#ifdef BUILD_WITH_REDIS
  redis::ConnectionSettings* CreateREDISConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_MEMCACHED
  memcached::ConnectionSettings* CreateMEMCACHEDConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_SSDB
  ssdb::ConnectionSettings* CreateSSDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_LEVELDB
  leveldb::ConnectionSettings* CreateLEVELDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_ROCKSDB
  rocksdb::ConnectionSettings* CreateROCKSDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_UNQLITE
  unqlite::ConnectionSettings* CreateUNQLITEConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_LMDB
  lmdb::ConnectionSettings* CreateLMDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_UPSCALEDB
  upscaledb::ConnectionSettings* CreateUPSCALEDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_FORESTDB
  forestdb::ConnectionSettings* CreateFORESTDBConnection(const connection_path_t& connection_path) const;
#endif

#ifdef BUILD_WITH_PIKA
  pika::ConnectionSettings* CreatePIKAConnection(const connection_path_t& connection_path) const;
#endif
  std::string GetLoggingDirectory() const;
  void SetLoggingDirectory(const std::string& dir);

 private:
  ConnectionSettingsFactory();

  std::string logging_dir_;
};

}  // namespace proxy
}  // namespace fastonosql
