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

#include "core/ssdb/ssdb_database.h"

#include "core/iserver.h"

namespace fastonosql {
namespace ssdb {

SsdbDatabase::SsdbDatabase(IServerSPtr server, IDataBaseInfoSPtr info)
  : IDatabase(server, info) {
  DCHECK(server);
  DCHECK(info);
  DCHECK(server->type() == SSDB);
  DCHECK(info->type() == SSDB);
}

}
}  // namespace fastonosql
