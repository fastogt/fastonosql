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

#include "proxy/database/idatabase.h"

#include "proxy/server/iserver.h"

namespace fastonosql {
namespace proxy {

IDatabase::IDatabase(IServerSPtr server, core::IDataBaseInfoSPtr info) : info_(info), server_(server) {
  CHECK(server);
  CHECK(info);
}

IDatabase::~IDatabase() {}

IServerSPtr IDatabase::GetServer() const {
  return server_;
}

bool IDatabase::IsDefault() const {
  return info_->IsDefault();
}

core::db_name_t IDatabase::GetName() const {
  return info_->GetName();
}

void IDatabase::LoadContent(const events_info::LoadDatabaseContentRequest& req) {
  DCHECK_EQ(req.inf, info_);

  server_->LoadDatabaseContent(req);
}

core::IDataBaseInfoSPtr IDatabase::GetInfo() const {
  return info_;
}

void IDatabase::Execute(const events_info::ExecuteInfoRequest& req) {
  server_->Execute(req);
}

}  // namespace proxy
}  // namespace fastonosql
