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

#pragma once

#include <string>

#include <common/net/types.h>
#include <common/patterns/singleton_pattern.h>

#include <fastonosql/core/connection_types.h>

#include "proxy/connection_settings/connection_settings_path.h"
#include "proxy/connection_settings/types.h"

namespace fastonosql {
namespace proxy {

extern const char kMagicNumber;
extern const char kSettingValueDelemiter;

class IConnectionSettings;
class IConnectionSettingsBase;
class IConnectionSettingsRemote;

class ConnectionSettingsFactory : public common::patterns::LazySingleton<ConnectionSettingsFactory> {
 public:
  friend class common::patterns::LazySingleton<ConnectionSettingsFactory>;

  serialize_t ConvertSettingsToString(IConnectionSettings* settings);
  serialize_t ConvertSettingsToString(IConnectionSettingsBase* settings);

  IConnectionSettingsBase* CreateSettingsFromTypeConnection(core::ConnectionType type,
                                                            const connection_path_t& connection_path);
  IConnectionSettingsBase* CreateSettingsFromString(const serialize_t& value);  // can return nullptr

  IConnectionSettingsRemote* CreateRemoteSettingsFromTypeConnection(core::ConnectionType type,
                                                                    const connection_path_t& connection_path,
                                                                    const common::net::HostAndPort& host);

  std::string GetLoggingDirectory() const;
  void SetLoggingDirectory(const std::string& dir);

 private:
  ConnectionSettingsFactory();

  std::string logging_dir_;
};

}  // namespace proxy
}  // namespace fastonosql
