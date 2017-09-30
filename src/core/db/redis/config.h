/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "core/config/config.h"  // for RemoteConfig

namespace fastonosql {
namespace core {
namespace redis {

struct Config : public RemoteConfig {
  enum { db_num_default = 0 };
  Config();

  std::string hostsocket;
  int db_num;
  std::string auth;
  bool is_ssl;
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql

namespace common {
std::string ConvertToString(const fastonosql::core::redis::Config& conf);
bool ConvertFromString(const std::string& from, fastonosql::core::redis::Config* out);
}  // namespace common
