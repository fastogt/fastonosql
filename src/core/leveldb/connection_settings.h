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

#include "core/connection_settings/iconnection_settings_local.h"

#include "core/leveldb/config.h"

namespace fastonosql {
namespace core {
namespace leveldb {

class ConnectionSettings : public IConnectionSettingsLocal {
 public:
  explicit ConnectionSettings(const connection_path_t& connectionName);

  virtual BaseConfig Conf() const override;
  virtual LocalConfig LocalConf() const override;

  virtual std::string CommandLine() const override;
  virtual void SetCommandLine(const std::string& line) override;

  Config Info() const;
  void SetInfo(const Config& info);

  virtual std::string FullAddress() const override;

  virtual ConnectionSettings* Clone() const override;

 private:
  Config info_;
};

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
