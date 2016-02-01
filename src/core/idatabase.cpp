/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "core/idatabase.h"

#include <string>

#include "core/iserver.h"

namespace fastonosql {

IDatabase::IDatabase(IServerSPtr server, DataBaseInfoSPtr info)
  : info_(info), server_(server) {
  DCHECK(server);
  DCHECK(info);
  DCHECK(server->type() == info->type());
}

IDatabase::~IDatabase() {
}

connectionTypes IDatabase::type() const {
  return server_->type();
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

void IDatabase::loadContent(const EventsInfo::LoadDatabaseContentRequest& req) {
  DCHECK(req.inf_ == info_);
  server_->loadDatabaseContent(req);
}

void IDatabase::setDefault(const EventsInfo::SetDefaultDatabaseRequest& req) {
  DCHECK(req.inf_ == info_);
  server_->setDefaultDb(req);
}

DataBaseInfoSPtr IDatabase::info() const {
  return info_;
}

void IDatabase::setInfo(DataBaseInfoSPtr info) {
  info_ = info;
}

void IDatabase::executeCommand(const EventsInfo::CommandRequest& req) {
  DCHECK(req.inf_ == info_);
  server_->executeCommand(req);
}

}  // namespace fastonosql
