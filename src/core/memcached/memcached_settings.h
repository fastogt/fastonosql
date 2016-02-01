/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>

#include "core/connection_settings.h"

#include "core/memcached/memcached_config.h"

namespace fastonosql {

class MemcachedConnectionSettings
      : public IConnectionSettingsRemote {
 public:
  explicit MemcachedConnectionSettings(const std::string& connectionName);

  virtual std::string commandLine() const;
  virtual void setCommandLine(const std::string& line);

  virtual void setHost(const common::net::hostAndPort& host);
  virtual common::net::hostAndPort host() const;

  memcachedConfig info() const;
  void setInfo(const memcachedConfig& info);

  virtual IConnectionSettings* clone() const;

 private:
  virtual std::string toCommandLine() const;
  virtual void initFromCommandLine(const std::string& val);
  memcachedConfig info_;
};

}  // namespace fastonosql
