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

#pragma once

#include <string>  // for string

#include <common/net/types.h>  // for HostAndPort

#include "core/connection_settings/connection_settings.h"
#include "core/memcached/config.h"  // for Config

namespace fastonosql {
namespace core {
namespace memcached {

class ConnectionSettings : public IConnectionSettingsRemote {
 public:
  explicit ConnectionSettings(const connection_path_t& connectionName);

  virtual std::string commandLine() const;
  virtual void setCommandLine(const std::string& line);

  virtual void setHost(const common::net::HostAndPort& host);
  virtual common::net::HostAndPort host() const;

  Config info() const;
  void setInfo(const Config& info);

  virtual ConnectionSettings* Clone() const;

 private:
  Config info_;
};

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
