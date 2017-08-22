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

#include "core/database/idatabase_info.h"

#include "proxy/proxy_fwd.h"

namespace fastonosql {
namespace proxy {

namespace events_info {
struct ExecuteInfoRequest;
struct LoadDatabaseContentRequest;
}  // namespace events_info

class IDatabase {
 public:
  virtual ~IDatabase();

  IServerSPtr GetServer() const;
  core::IDataBaseInfoSPtr GetInfo() const;

  core::connectionTypes GetType() const;
  bool IsDefault() const;
  std::string GetName() const;

  void LoadContent(const events_info::LoadDatabaseContentRequest& req);
  void Execute(const events_info::ExecuteInfoRequest& req);

 protected:
  IDatabase(IServerSPtr server, core::IDataBaseInfoSPtr info);

 private:
  const core::IDataBaseInfoSPtr info_;
  const IServerSPtr server_;
};

}  // namespace proxy
}  // namespace fastonosql
