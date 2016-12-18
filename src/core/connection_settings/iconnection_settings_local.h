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

#include "core/connection_settings/iconnection_settings.h"
#include "core/connection_types.h"  // for connectionTypes

namespace fastonosql {
namespace core {

class IConnectionSettingsLocal : public IConnectionSettingsBase {
 public:
  virtual std::string FullAddress() const override;

  virtual std::string Delimiter() const override = 0;
  virtual void SetDelimiter(const std::string& delimiter) override = 0;

  virtual std::string NsSeparator() const override = 0;
  virtual void SetNsSeparator(const std::string& ns) override = 0;

  virtual std::string DBPath() const = 0;
  virtual void SetDBPath(const std::string& db_path) = 0;

  virtual std::string CommandLine() const override = 0;
  virtual void SetCommandLine(const std::string& line) override = 0;

  virtual IConnectionSettingsBase* Clone() const override = 0;

 protected:
  IConnectionSettingsLocal(const connection_path_t& connectionPath, connectionTypes type);
};

}  // namespace core
}  // namespace fastonosql
