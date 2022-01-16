/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "proxy/connection_settings/iconnection_settings.h"

namespace fastonosql {
namespace proxy {

class IConnectionSettingsLocal : public IConnectionSettingsBase {
 public:
  std::string GetFullAddress() const override;

  std::string GetDelimiter() const override = 0;
  void SetDelimiter(const std::string& delimiter) override = 0;

  virtual std::string GetDBPath() const = 0;
  virtual void SetDBPath(const std::string& db_path) = 0;

  std::string GetCommandLine() const override = 0;
  void SetCommandLine(const std::string& line) override = 0;

  IConnectionSettingsBase* Clone() const override = 0;

 protected:
  IConnectionSettingsLocal(const connection_path_t& connection_path,
                           const std::string& log_directory,
                           core::ConnectionType type);
};

}  // namespace proxy
}  // namespace fastonosql
