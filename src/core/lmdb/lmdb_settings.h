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

#include <string>

#include "core/connection_settings.h"

#include "core/lmdb/lmdb_config.h"

namespace fastonosql {
namespace lmdb {

class LmdbConnectionSettings
  : public IConnectionSettingsBase {
 public:
  explicit LmdbConnectionSettings(const std::string& connectionName);

  virtual std::string commandLine() const;
  virtual void setCommandLine(const std::string& line);

  lmdbConfig info() const;
  void setInfo(const lmdbConfig &info);

  virtual std::string fullAddress() const;

  virtual IConnectionSettings* clone() const;

 private:
  virtual std::string toCommandLine() const;
  virtual void initFromCommandLine(const std::string& val);
  lmdbConfig info_;
};

}  // namespace lmdb
}  // namespace fastonosql
