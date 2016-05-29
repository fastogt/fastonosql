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

#include "core/core_fwd.h"
#include "core/events/events_info.h"

namespace fastonosql {
namespace core {

class IDatabase {
 public:
  virtual ~IDatabase();

  IServerSPtr server() const;
  IDataBaseInfoSPtr info() const;

  connectionTypes type() const;
  bool isDefault() const;
  std::string name() const;

  void loadContent(const events_info::LoadDatabaseContentRequest& req);
  void setDefault(const events_info::SetDefaultDatabaseRequest& req);

  void executeCommand(const events_info::CommandRequest& req);
  void removeAllKeys(const events_info::ClearDatabaseRequest& req);

 protected:
  IDatabase(IServerSPtr server, IDataBaseInfoSPtr info);

 private:
  const IDataBaseInfoSPtr info_;
  const IServerSPtr server_;
};

}  // namespace core
}  // namespace fastonosql
