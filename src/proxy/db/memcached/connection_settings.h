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

#include "proxy/connection_settings/iconnection_settings_remote.h"

#include <fastonosql/core/db/memcached/config.h>  // for Config

namespace fastonosql {
namespace proxy {
namespace memcached {

class ConnectionSettings : public IConnectionSettingsRemote {
 public:
  ConnectionSettings(const connection_path_t& connection_path, const std::string& log_directory);

  core::memcached::Config GetInfo() const;
  void SetInfo(const core::memcached::Config& info);

  virtual std::string GetDelimiter() const override;
  virtual void SetDelimiter(const std::string& delimiter) override;

  virtual common::net::HostAndPort GetHost() const override;
  virtual void SetHost(const common::net::HostAndPort& host) override;

  virtual std::string GetCommandLine() const override;
  virtual void SetCommandLine(const std::string& line) override;

  virtual ConnectionSettings* Clone() const override;

 private:
  core::memcached::Config info_;
};

}  // namespace memcached
}  // namespace proxy
}  // namespace fastonosql
