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

#include <stddef.h>  // for size_t
#include <string>    // for string

#include "proxy/database/idatabase.h"  // for IDatabase
#include "proxy/proxy_fwd.h"           // for IServerSPtr

namespace fastonosql {
namespace proxy {
namespace lmdb {

class Database : public IDatabase {
 public:
  Database(IServerSPtr server, core::IDataBaseInfoSPtr info);
};

}  // namespace lmdb
}  // namespace proxy
}  // namespace fastonosql
