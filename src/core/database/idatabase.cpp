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

#include "core/database/idatabase.h"

#include <memory>  // for __shared_ptr
#include <string>  // for string

#include <common/macros.h>  // for DCHECK_EQ, CHECK

#include "core/events/events_info.h"  // for ClearDatabaseRequest, etc
#include "core/server/iserver.h"      // for IServer

namespace fastonosql {
namespace core {

IDatabase::IDatabase(IServerSPtr server, IDataBaseInfoSPtr info) : info_(info), server_(server) {
  CHECK(server);
  CHECK(info);
  CHECK(server->type() == info->type());
}

IDatabase::~IDatabase() {}

connectionTypes IDatabase::type() const {
  return info_->type();
}

IServerSPtr IDatabase::server() const {
  return server_;
}

bool IDatabase::isDefault() const {
  return info_->isDefault();
}

std::string IDatabase::name() const {
  return info_->name();
}

void IDatabase::loadContent(const events_info::LoadDatabaseContentRequest& req) {
  DCHECK_EQ(req.inf, info_);

  server_->loadDatabaseContent(req);
}

void IDatabase::setDefault(const events_info::SetDefaultDatabaseRequest& req) {
  DCHECK_EQ(req.inf, info_);

  server_->setDefaultDB(req);
}

IDataBaseInfoSPtr IDatabase::info() const {
  return info_;
}

translator_t IDatabase::Translator() const {
  return server_->Translator();
}

void IDatabase::execute(const events_info::ExecuteInfoRequest& req) {
  server_->execute(req);
}

void IDatabase::removeAllKeys(const events_info::ClearDatabaseRequest& req) {
  DCHECK_EQ(req.inf, info_);

  server_->clearDB(req);
}

}  // namespace core
}  // namespace fastonosql
