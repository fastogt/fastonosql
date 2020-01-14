/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "proxy/connection_settings/iconnection_settings_local.h"

#include <fastonosql/core/db/forestdb/config.h>

namespace fastonosql {
namespace proxy {
namespace forestdb {

class ConnectionSettings : public IConnectionSettingsLocal {
 public:
  explicit ConnectionSettings(const connection_path_t& connection_path, const std::string& log_directory);

  core::forestdb::Config GetInfo() const;
  void SetInfo(const core::forestdb::Config& info);

  std::string GetDelimiter() const override;
  void SetDelimiter(const std::string& delimiter) override;

  std::string GetDBPath() const override;
  void SetDBPath(const std::string& db_path) override;

  std::string GetCommandLine() const override;
  void SetCommandLine(const std::string& line) override;

  ConnectionSettings* Clone() const override;

 private:
  core::forestdb::Config info_;
};

}  // namespace forestdb
}  // namespace proxy
}  // namespace fastonosql
