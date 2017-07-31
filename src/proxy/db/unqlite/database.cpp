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

#include "proxy/db/unqlite/database.h"

#include <memory>  // for __shared_ptr

#include <common/macros.h>  // for DCHECK

#include "core/connection_types.h"  // for core::connectionTypes::UNQLITE

namespace fastonosql {
namespace proxy {
namespace unqlite {

Database::Database(IServerSPtr server, core::IDataBaseInfoSPtr info) : IDatabase(server, info) {
  DCHECK(server);
  DCHECK(info);
  DCHECK(info->Type() == core::UNQLITE);
}

}  // namespace unqlite
}  // namespace proxy
}  // namespace fastonosql
