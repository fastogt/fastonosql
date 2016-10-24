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

#include "core/lmdb/connection_settings.h"

#include <string>  // for string

#include <common/convert2string.h>  // for ConvertFromString

#include "core/connection_types.h"  // for connectionTypes::LMDB

namespace fastonosql {
namespace core {
namespace lmdb {

ConnectionSettings::ConnectionSettings(const connection_path_t& connectionName)
    : IConnectionSettingsLocal(connectionName, LMDB), info_() {}

std::string ConnectionSettings::DBpath() const {
  return info_.dbname;
}

std::string ConnectionSettings::CommandLine() const {
  return common::ConvertToString(info_);
}

void ConnectionSettings::SetCommandLine(const std::string& line) {
  info_ = common::ConvertFromString<Config>(line);
}

Config ConnectionSettings::Info() const {
  return info_;
}

void ConnectionSettings::SetInfo(const Config& info) {
  info_ = info;
}

std::string ConnectionSettings::FullAddress() const {
  return info_.dbname;
}

ConnectionSettings* ConnectionSettings::Clone() const {
  ConnectionSettings* red = new ConnectionSettings(*this);
  return red;
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
