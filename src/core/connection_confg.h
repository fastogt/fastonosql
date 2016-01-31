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

#include <vector>
#include <string>

namespace fastonosql {

enum ConfigType {
  LOCAL,
  REMOTE
};

// -d
template<ConfigType ctype>
struct BaseConfig {
  BaseConfig()
    : mb_delim_("\n") {
  }

  ConfigType type() const {
    return ctype;
  }

  std::string mb_delim_;
};

// -f
struct LocalConfig
        : public BaseConfig<LOCAL> {
  explicit LocalConfig(const std::string& dbname);

  std::vector<std::string> args() const;

  std::string dbname_;
};

// -h -p
struct RemoteConfig
        : public BaseConfig<REMOTE> {
  RemoteConfig(const std::string& hostip, uint16_t port);

  std::vector<std::string> args() const;

  std::string hostip_;
  uint16_t hostport_;
};

}
